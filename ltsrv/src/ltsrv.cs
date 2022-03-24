using System;
using System.Collections.Generic;
using LamestWebserver;
using LamestWebserver.UI;
using LamestWebserver.Serialization;

public class ltsrv
{
  public static Analysis _Analysis;

  [STAThread]
  public static void Main(string[] args)
  {
    _Analysis = Serializer.ReadJsonData<Analysis>(args[0]);

    Console.WriteLine("Deserialized.");
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
