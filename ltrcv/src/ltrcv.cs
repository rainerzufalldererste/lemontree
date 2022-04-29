using System;
using System.Collections.Generic;
using System.Security.Cryptography;
using System.Linq;
using System.Net.Sockets;
using System.IO;
using System.Threading;
using System.Text;
using static Monocypher.Monocypher;

namespace ltrcv
{
  public class ltrcv
  {
    static ushort port = 0x2E11;
    static int readTimeout = 180000;
    static int maxThreads = 256;
    
    static int threadCount = 0;
    static Mutex threadCountMutex = new Mutex();

    static RandomNumberGenerator csprng = RandomNumberGenerator.Create();
    static Mutex csprngMutex = new Mutex();

    static Thread serverThread;
    static bool keepRunning = true;
    static TcpListener tcpListener;

    static string[] recognizedProductNames;
    static byte[] serverPrivateKey;
    static byte[] serverPublicKey;

    [STAThread]
    public static void Main(string[] args)
    {
      const string recognizedProductNamesFile = "productNames.txt";
      Console.WriteLine($"Attempting to read {nameof(recognizedProductNames)} from '{recognizedProductNamesFile}'...");
      recognizedProductNames = (from x in File.ReadAllLines(recognizedProductNamesFile) where !string.IsNullOrWhiteSpace(x) select x.Replace("\r", "").Replace("\n", "")).ToArray();

      Console.WriteLine("\nProduct Names:\n");
      
      foreach (var x in recognizedProductNames)
        Console.WriteLine(x);

      Console.WriteLine();

      Console.WriteLine("Attempting to load private key...");

      serverPrivateKey = File.ReadAllBytes("C:\\Windows\\cert\\ltrcv.bin");
      crypto_x25519_public_key(serverPublicKey, serverPrivateKey); // generate public key.

      tcpListener = new TcpListener(port);

      serverThread = new Thread(() => { RcvThread(); });
      serverThread.Start();

      do
      {
        Console.WriteLine("Enter 'exit' to quit.");
      } while (Console.ReadLine() != "exit");

      keepRunning = false;
    }

    static private TcpClient AcceptClient()
    {
      var receiveTask = tcpListener.AcceptTcpClientAsync();
      
      while (!receiveTask.Wait(100))
        if (!keepRunning)
          return null;

      TcpClient tcpClient = receiveTask.Result;
      tcpClient.ReceiveTimeout = readTimeout;

      Console.WriteLine("Client Connected: " + tcpClient.Client.RemoteEndPoint.ToString());

      return tcpClient;
    }

    static void RcvThread()
    {
      try
      {
        tcpListener.Start();
      }
      catch (Exception e)
      {
        Console.WriteLine($"Failed to start TCP Listener. ({e.Message})");
        return;
      }

      while (keepRunning)
      {
        int threads = maxThreads;

        if (threadCountMutex.WaitOne())
        {
          threads = threadCount;
          threadCountMutex.ReleaseMutex();
        }

        if (threads < maxThreads)
        {
          try
          {
            TcpClient client = AcceptClient();

            if (client == null)
              continue;

            Console.WriteLine($"TCP Client connected. ({client.Client.RemoteEndPoint})");

            new Thread(() => { ClientThread(client); }).Start();
          }
          catch (Exception e)
          {
            Console.WriteLine($"Failed to accept TCP Client. ({e.Message})");
          }
        }
        else
        {
          Thread.Sleep(10);
        }
      }
    }

    static void ClientThread(TcpClient client)
    {
      if (!threadCountMutex.WaitOne())
        return;

      threadCount++;

      threadCountMutex.ReleaseMutex();

      NetworkStream stream = null;

      try
      {
        stream = client.GetStream();

        string productName;

        // Handshake Step 1: Receive & Validate Product Name.
        if (keepRunning)
        {
          byte[] bytes = new byte[255];

          int bytesRead = stream.Read(bytes, 0, bytes.Length);

          productName = Encoding.UTF8.GetString(bytes, 0, bytesRead);

          if (!recognizedProductNames.Contains(productName))
          {
            return;
          }
        }

        byte[] challengeBytes = new byte[8 + 3];

        // Handshake Step 2: Send Challenge Data.
        if (keepRunning)
        {
          if (!csprngMutex.WaitOne())
            return;

          try
          {
            csprng.GetBytes(challengeBytes);
          }
          catch (Exception e)
          {
            Console.WriteLine($"Failed to generate random bytes. ({e.Message})");
            return;
          }
          finally
          {
            csprngMutex.ReleaseMutex();
          }

          stream.Write(challengeBytes, 0, challengeBytes.Length);
        }

        byte[] proposedSolution = new byte[1024];

        // Handshake Step 3: Retrieve Solution.
        {
          for (int i = 0; i < 8; i++)
            proposedSolution[i] = challengeBytes[i];

          int bytesRead = stream.Read(proposedSolution, 8, proposedSolution.Length - 8);

          if (bytesRead != proposedSolution.Length - 8)
          {
            return;
          }
        }

        // Validate Solution.
        if (keepRunning)
        {
          var sha512 = SHA512.Create();

          byte[] hash = sha512.ComputeHash(proposedSolution);

          for (int i = 0; i < 3; i++)
          {
            if (hash[8 + i] != challengeBytes[13 + i])
            {
              return;
            }
          }
        }

        // Handshake Step 4, 5: Receive Client Public Key. Receive MAC. Then: Receive Data, Decrypt Data, Validate, Save.
        {
          byte[] clientPublicKey = new byte[32];

          int bytesRead = stream.Read(clientPublicKey, 0, clientPublicKey.Length);

          if (bytesRead != clientPublicKey.Length)
          {
            return;
          }

          Span<byte> serverPublicKey = new byte[32];
          Span<byte> sharedSecret = new byte[32];

          crypto_x25519(sharedSecret, serverPrivateKey, clientPublicKey); // generate shared secret.

          Span<byte> sessionKeys = new byte[64];

          // Hash keys & shared secret to `sessionKeys`.
          {
            crypto_blake2b_ctx ctx = new crypto_blake2b_ctx();

            crypto_blake2b_init(ref ctx);
            crypto_blake2b_update(ref ctx, sharedSecret);
            crypto_blake2b_update(ref ctx, clientPublicKey);
            crypto_blake2b_update(ref ctx, serverPublicKey);
            crypto_blake2b_final(ref ctx, sessionKeys);
          }

          byte[] mac = new byte[16];

          bytesRead = stream.Read(mac, 0, mac.Length);

          if (bytesRead != mac.Length)
          {
            return;
          }

          byte[] data = new byte[1024 * 128];

          bytesRead = stream.Read(data, 0, data.Length);

          if (bytesRead == 0)
          {
            return;
          }

          Span<byte> actualData = ((Span<byte>)data).Slice(0, bytesRead);

          if (0 != crypto_unlock(actualData, sessionKeys.Slice(0, 32), sessionKeys.Slice(32, 24), mac, actualData)) // yes, we're inplace decrypting. according to the monocypher documentation this is fully supported. I don't see how a c# binding would change that.
          {
            return; // Content is invalid.
          }

          // TODO: Parse (to see if the project name is valid), then save in the appropriate folder.
        }
      }
      catch (Exception e)
      {
        Console.WriteLine($"Failed to handle client. ({e.Message})");
      }
      finally
      {
        try
        {
          if (stream != null)
            stream.Close();
          
          client.Close();
        }
        catch { }

        if (threadCountMutex.WaitOne())
        {
          threadCount--;

          threadCountMutex.ReleaseMutex();
        }
      }
    }
  }
}
