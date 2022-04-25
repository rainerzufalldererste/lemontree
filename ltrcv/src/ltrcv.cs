using System;
using System.Collections.Generic;
using System.Security.Cryptography;
using System.Linq;
using System.Net.Sockets;
using System.IO;
using System.Threading;
using System.Text;

namespace ltrcv
{
  public class ltrcv
  {
    static ushort port = 0x2E11;
    static int readTimeout = 20000;
    static int maxThreads = 256;
    
    static int threadCount = 0;
    static Mutex threadCountMutex = new Mutex();

    static RandomNumberGenerator csprng = RandomNumberGenerator.Create();
    static Mutex csprngMutex = new Mutex();

    static Thread serverThread;
    static bool keepRunning = true;
    static TcpListener tcpListener;

    [STAThread]
    public static void Main(string[] args)
    {
      tcpListener = new TcpListener(port);

      serverThread = new Thread(() => { RcvThread(); });
      serverThread.Start();
    }

    static private TcpClient AcceptClient()
    {
      var receiveTask = tcpListener.AcceptTcpClientAsync();
      receiveTask.Wait();

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
            var client = AcceptClient();

            Console.WriteLine($"TCP Client connected. ({client.Client.RemoteEndPoint})");
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

      try
      {
        if (!csprngMutex.WaitOne())
          return;

        byte[] bytes = new byte[6 + 8];

        try
        {
          csprng.GetBytes(bytes);
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

        // ...
      }
      catch (Exception e)
      {
        Console.WriteLine($"Failed to handle client. ({e.Message})");
      }
      finally
      {
        if (threadCountMutex.WaitOne())
        {
          threadCount--;

          threadCountMutex.ReleaseMutex();
        }
      }
    }
  }
}
