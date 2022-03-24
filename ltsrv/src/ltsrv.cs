using System;
using System.Collections.Generic;
using LamestWebserver;
using LamestWebserver.UI;
using LamestWebserver.Serialization;

public class ltsrv
{
  public static Analysis _Analysis;
  public static Info _Info;

  [STAThread]
  public static void Main(string[] args)
  {
    _Analysis = Serializer.ReadJsonData<Analysis>(args[0]);

    Console.WriteLine("Deserialized Analysis.");

    for (int i = 1; i < args.Length;)
    {
      switch (args[i])
      {
        case "--info":
          if (i + 1 >= args.Length)
          {
            Console.WriteLine($"Argument '{args[i]}' is missing a filepath.");
            throw new Exception();
          }

          _Info = Serializer.ReadJsonData<Info>(args[i + 1]);

          if (_Info.productName != _Analysis.productName)
          {
            Console.WriteLine($"Product Name Mismatch '{_Info.productName}' != '{_Analysis.productName}'.");
            throw new Exception();
          }

          i += 2;

          break;

        default:
          Console.WriteLine($"Unexpected Argument '{args[i]}'.");
          throw new Exception();
      }
    }

    foreach (var subSystem in _Analysis.states)
    {
      foreach (var state in subSystem.value)
      {

      }
    }
  }
}

public struct uint64_t
{
  internal ulong value;

  public uint64_t(ulong v)
  {
    value = v;
  }

  public static explicit operator uint64_t(string s)
  {
    ulong value;

    if (s.StartsWith("0x"))
      value = ulong.Parse(s.Substring(2), System.Globalization.NumberStyles.HexNumber);
    else
      value = ulong.Parse(s);

    return new uint64_t(value);
  }

  public static explicit operator uint64_t(ulong v) => new uint64_t(v);
  public static explicit operator uint64_t(int v) => new uint64_t((ulong)v);
}

public interface IPage
{
  HContainer GetPageContents();
}

public class Analysis
{
  public string productName;
  public uint64_t majorVersion, minorVersion;
  public List<Ref<uint64_t, List<Ref<StateId, State>>>> states;
  public List<Ref<uint64_t, List<Ref<uint64_t, Operation>>>> operations;
}

public class Ref<Index, Value>
{
  public Index index;
  public Value value;
}

public struct StateId
{
  public uint64_t state, subState;
}

public class TransitionData
{
  public double avgDelay, maxDelay, minDelay;
  public uint64_t count;
}

public class State : TransitionData
{
  public double avgStartDelay;
  public List<Ref<StateId, TransitionData>> nextState;
  public List<Ref<StateId, TransitionData>> previousState;
  public List<Ref<uint64_t, TransitionData>> operations;
  public List<Ref<uint64_t, TransitionData>> previousOperation;
  public List<Ref<StateId, TransitionData>> stateReach;
  public List<Ref<uint64_t, TransitionData>> operationReach;
}

public class Operation : TransitionData
{
  public double avgStartDelay;
  public List<Ref<StateId, TransitionData>> parentState;
  public List<Ref<StateId, TransitionData>> nextState;
  public List<Ref<uint64_t, TransitionData>> nextOperation;
  public List<Ref<uint64_t, TransitionData>> lastOperation;
  public List<Ref<uint64_t, uint64_t>> operationIndexCount;
}

public class Info
{
  public string productName;
  public List<Ref<uint64_t, List<string>>> states;
  public List<Ref<uint64_t, List<string>>> operations;
}
