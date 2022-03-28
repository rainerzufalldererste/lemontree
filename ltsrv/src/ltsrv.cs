using System;
using System.Collections.Generic;
using System.Linq;
using LamestWebserver;
using LamestWebserver.UI;
using LamestWebserver.Core;
using LamestWebserver.Serialization;

public class AnalysisInfo
{
  public Analysis analysis;
  public Info info;

  public AnalysisInfo(Analysis a)
  {
    analysis = a;
  }
}

public class ltsrv
{
  public static Dictionary<string, Dictionary<uint64_t, Dictionary<uint64_t, AnalysisInfo>>> _Analysis = new Dictionary<string, Dictionary<uint64_t, Dictionary<uint64_t, AnalysisInfo>>>();

  [STAThread]
  public static void Main(string[] args)
  {
    System.Globalization.CultureInfo.CurrentCulture = System.Globalization.CultureInfo.InvariantCulture;

    var analysis = Serializer.ReadJsonData<Analysis>(args[0]);

    Console.WriteLine("Deserialized Analysis.");

    analysis.Sort();

    Console.WriteLine("Sorted Analysis.");

    _Analysis.Add(analysis.productName, new Dictionary<uint64_t, Dictionary<uint64_t, AnalysisInfo>> { { analysis.majorVersion, new Dictionary<uint64_t, AnalysisInfo>() { { analysis.minorVersion, new AnalysisInfo(analysis) } } } });

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

          var info = Serializer.ReadJsonData<Info>(args[i + 1]);
          var container = _Analysis[info.productName][info.majorVersion];

          if (info.minorVersion.HasValue)
            container[info.minorVersion.Value].info = info;
          else
            foreach (var x in container)
              x.Value.info = info;

          i += 2;

          Console.WriteLine("Deserialized Info.");

          break;

        default:
          Console.WriteLine($"Unexpected Argument '{args[i]}'.");
          throw new Exception();
      }
    }

    using (var ws = new WebServer(8080, "web"))
    {
      Master.DiscoverPages();

      while (Console.ReadLine() != "exit")
        Console.WriteLine("Enter 'exit' to quit.");
    }
  }
  public static HElement GetPage(string title, IEnumerable<HElement> elements)
  {
    return new PageBuilder(title) { Elements = { new HContainer { Class = "main", Elements = { new HHeadline(title) { Class = "Page" }, new HContainer(elements) { Class = "inner" } } } }, StylesheetLinks = { "style.css" } };
  }
}

public class SubSystemInfo : ElementResponse
{
  public SubSystemInfo() : base("subsystem") { }

  protected override HElement GetElement(SessionData sessionData) => GetMenu(sessionData);

  internal static HElement GetMenu(SessionData sessionData)
  {
    string productName = sessionData.HttpHeadVariables["p"];
    ulong majorVersion, minorVersion, subSystem;

    if (productName == null)
      return ltsrv.GetPage("Products", ShowProjects(sessionData));
    else if (!ulong.TryParse(sessionData.HttpHeadVariables["V"], out majorVersion))
      return ltsrv.GetPage("Sub Systems", ShowMajorVersions(sessionData, productName));
    else if (!ulong.TryParse(sessionData.HttpHeadVariables["v"], out minorVersion))
      return ltsrv.GetPage("Sub Systems", ShowMinorVersions(sessionData, productName, majorVersion));
    else if (!ulong.TryParse(sessionData.HttpHeadVariables["ss"], out subSystem))
      return ltsrv.GetPage("Sub Systems", ShowSubSystems(sessionData, productName, majorVersion, minorVersion));
    else
      return ltsrv.GetPage("Sub System Info", ShowContents(sessionData, productName, majorVersion, minorVersion, subSystem));
  }

  internal static IEnumerable<HElement> ShowProjects(SessionData sessionData)
  {
    yield return new HHeadline($"Products");

    foreach (var x in ltsrv._Analysis)
      yield return new HLink(x.Key, $"/subsystem?p={x.Key}") { Class = "LargeButton" };
  }

  internal static IEnumerable<HElement> ShowMajorVersions(SessionData sessionData, string productName)
  {
    if (ltsrv._Analysis[productName].Count == 1)
    {
      yield return new HScript(ScriptCollection.GetPageReferalToX, $"/subsystem?p={productName.EncodeUrl()}&V={ltsrv._Analysis[productName].First().Key}");
      yield break;
    }

    yield return new HHeadline($"MajorVersion");

    foreach (var x in ltsrv._Analysis[productName])
      yield return new HLink(x.Key.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={x.Key}") { Class = "LargeButton" };
  }

  internal static IEnumerable<HElement> ShowMinorVersions(SessionData sessionData, string productName, ulong majorVersion)
  {
    if (ltsrv._Analysis[productName][(uint64_t)majorVersion].Count == 1)
    {
      yield return new HScript(ScriptCollection.GetPageReferalToX, $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}&v={ltsrv._Analysis[productName][(uint64_t)majorVersion].First().Key}");
      yield break;
    }

    yield return new HHeadline($"MinorVersion");

    foreach (var x in ltsrv._Analysis[productName][(uint64_t)majorVersion])
      yield return new HLink(x.Key.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}&v={x.Key}") { Class = "LargeButton" };
  }

  internal static IEnumerable<HElement> ShowSubSystems(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion)
  {
    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline($"'{productName}' (Version {majorVersion}.{minorVersion})");

    yield return new HHeadline($"SubSystems", 2);

    foreach (var x in analysis.subSystems)
      yield return new HLink(x.index.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={x.index}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Values U64", 2);

    foreach (var x in analysis.observedU64)
      yield return new HLink(info.GetValueNameU64(x.index.value), $"/value?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=u64&id={x.index}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Values I64", 2);

    foreach (var x in analysis.observedI64)
      yield return new HLink(info.GetValueNameI64(x.index.value), $"/value?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=i64&id={x.index}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Values String", 2);

    foreach (var x in analysis.observedString)
      yield return new HLink(info.GetValueNameString(x.index.value), $"/value?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=string&id={x.index}") { Class = "LargeButton" };
  }

  internal static IEnumerable<HElement> ShowContents(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong subSystem)
  {
    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline($"SubSystem {subSystem} of {productName} (Version {majorVersion}.{minorVersion})");

    yield return new HHeadline("States", 2);

    foreach (var x in analysis.subSystems.FindItem((uint64_t)subSystem).states)
      yield return new HLink(info.GetStateName((uint64_t)subSystem, x.index.state, x.index.subState), $"/state?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={subSystem}&id={x.index.state}&sid={x.index.subState}") { Class = "Button" };

    yield return new HHeadline("Operations", 2);

    foreach (var x in analysis.subSystems.FindItem((uint64_t)subSystem).operations)
      yield return new HLink(info.GetOperationName((uint64_t)subSystem, x.index), $"/p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={subSystem}&id={x.index}") { Class = "Button" };

    yield return analysis.ToHistorgramChart((uint64_t)subSystem, analysis.subSystems[(int)subSystem].value.profileData, "Performance", info);
  }
}

public class StateInfo : ElementResponse
{
  public StateInfo() : base("state") { }

  protected override HElement GetElement(SessionData sessionData)
  {
    string productName = sessionData.HttpHeadVariables["p"];
    ulong majorVersion, minorVersion, subSystem, stateIndex = 0, subStateIndex = 0;

    if (productName == null || !ulong.TryParse(sessionData.HttpHeadVariables["ss"], out subSystem) || !ulong.TryParse(sessionData.HttpHeadVariables["V"], out majorVersion) || !ulong.TryParse(sessionData.HttpHeadVariables["v"], out minorVersion) || !ulong.TryParse(sessionData.HttpHeadVariables["id"], out stateIndex) || !ulong.TryParse(sessionData.HttpHeadVariables["sid"], out subStateIndex))
    {
      return SubSystemInfo.GetMenu(sessionData);
    }
    else
    {
      return ltsrv.GetPage("State Info", ShowState(sessionData, productName, majorVersion, minorVersion, subSystem, stateIndex, subStateIndex));
    }
  }

  private IEnumerable<HElement> ShowState(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong subSystem, ulong state, ulong subStateIndex)
  {
    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    var s = analysis.subSystems.FindItem((uint64_t)subSystem).states.FindItem(new StateId(state, subStateIndex));

    yield return new HHeadline(info.GetStateName((uint64_t)subSystem, (uint64_t)state, (uint64_t)subStateIndex)) { Class = "stateName" };

    yield return analysis.DisplayInfo(s);
    yield return analysis.ToPieChart((uint64_t)subSystem, s.nextState, "Next State", info);
    yield return analysis.ToPieChart((uint64_t)subSystem, s.previousState, "Previous State", info);
    yield return analysis.ToPieChart((uint64_t)subSystem, s.operations, "Operations", info);
    yield return analysis.ToPieChart((uint64_t)subSystem, s.previousOperation, "Previous Operation", info);
    yield return analysis.ToHistorgramChart((uint64_t)subSystem, s.profileData, "Performance", info);
    yield return analysis.ToBarGraph((uint64_t)subSystem, s.stateReach, "State Reach", info);
    yield return analysis.ToBarGraph((uint64_t)subSystem, s.operationReach, "Operation Reach", info);
  }
}

public class OperationInfo : ElementResponse
{
  public OperationInfo() : base("op") { }

  protected override HElement GetElement(SessionData sessionData)
  {
    string productName = sessionData.HttpHeadVariables["p"];
    ulong majorVersion, minorVersion, subSystem, operationIndex = 0;

    if (productName == null || !ulong.TryParse(sessionData.HttpHeadVariables["ss"], out subSystem) || !ulong.TryParse(sessionData.HttpHeadVariables["V"], out majorVersion) || !ulong.TryParse(sessionData.HttpHeadVariables["v"], out minorVersion) || !ulong.TryParse(sessionData.HttpHeadVariables["id"], out operationIndex))
    {
      return SubSystemInfo.GetMenu(sessionData);
    }
    else
    {
      return ltsrv.GetPage("Operation Info", ShowOperation(sessionData, productName, majorVersion, minorVersion, subSystem, operationIndex));
    }
  }

  private IEnumerable<HElement> ShowOperation(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong subSystem, ulong operationIndex)
  {
    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    var s = analysis.subSystems.FindItem((uint64_t)subSystem).operations.FindItem((uint64_t)operationIndex);

    yield return new HHeadline(info.GetOperationName((uint64_t)subSystem, (uint64_t)operationIndex)) { Class = "stateName" };

    yield return analysis.DisplayInfo(s);
    yield return analysis.ToPieChart(s.operationIndexCount, "Operation Index");
    yield return analysis.ToPieChart((uint64_t)subSystem, s.nextOperation, "Next Operation", info);
    yield return analysis.ToPieChart((uint64_t)subSystem, s.parentState, "Parent State", info);
    yield return analysis.ToPieChart((uint64_t)subSystem, s.nextState, "Next State", info);
    yield return analysis.ToPieChart((uint64_t)subSystem, s.lastOperation, "Previous Operation", info);
  }
}

public class ValueInfo : ElementResponse
{
  public ValueInfo() : base("value") { }

  protected override HElement GetElement(SessionData sessionData)
  {
    string productName = sessionData.HttpHeadVariables["p"];
    ulong majorVersion, minorVersion, valueIndex = 0;
    string valueType = sessionData.HttpHeadVariables["t"];

    if (productName == null || valueType == null || !ulong.TryParse(sessionData.HttpHeadVariables["V"], out majorVersion) || !ulong.TryParse(sessionData.HttpHeadVariables["v"], out minorVersion) || !ulong.TryParse(sessionData.HttpHeadVariables["id"], out valueIndex))
    {
      return SubSystemInfo.GetMenu(sessionData);
    }
    else
    {
      switch (valueType)
      {
        case "u64": return ltsrv.GetPage("Value Info", ShowValueU64(sessionData, productName, majorVersion, minorVersion, valueIndex));
        case "i64": return ltsrv.GetPage("Value Info", ShowValueI64(sessionData, productName, majorVersion, minorVersion, valueIndex));
        case "string": return ltsrv.GetPage("Value Info", ShowValueString(sessionData, productName, majorVersion, minorVersion, valueIndex));
        default: return SubSystemInfo.GetMenu(sessionData);
      }
    }
  }

  private IEnumerable<HElement> ShowValueU64(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex)
  {
    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    var s = analysis.observedU64.FindItem((uint64_t)valueIndex);

    yield return new HHeadline(info.GetValueNameU64((uint64_t)valueIndex)) { Class = "stateName" };

    yield return analysis.DisplayInfo(s);
    yield return analysis.DisplayInfo(s.data);
    yield return analysis.ToPieChart(s.values, "Values", info, s.count);
  }

  private IEnumerable<HElement> ShowValueI64(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex)
  {
    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    var s = analysis.observedI64.FindItem((uint64_t)valueIndex);

    yield return new HHeadline(info.GetValueNameI64((uint64_t)valueIndex)) { Class = "stateName" };

    yield return analysis.DisplayInfo(s);
    yield return analysis.DisplayInfo(s.data);
    yield return analysis.ToPieChart(s.values, "Values", info, s.count);
  }

  private IEnumerable<HElement> ShowValueString(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex)
  {
    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    var s = analysis.observedString.FindItem((uint64_t)valueIndex);

    yield return new HHeadline(info.GetValueNameString((uint64_t)valueIndex)) { Class = "stateName" };

    yield return analysis.DisplayInfo(s.data);
    yield return analysis.ToPieChart(s.values, "Values", info, s.count);
  }
}

public static class ExtentionMethods
{
  public static T1 FindItem<T, T1>(this List<Ref<T, T1>> list, T index) where T : IEquatable<T>
  {
    foreach (var x in list)
      if (x.index.Equals(index))
        return x.value;

    throw new KeyNotFoundException();
  }

  public static string TryFindItem<T>(this List<Ref<T, string>> list, T index) where T : IEquatable<T>
  {
    foreach (var x in list)
      if (x.index.Equals(index))
        return x.value;

    return null;
  }
}

public struct uint64_t : IEquatable<uint64_t>, IComparable<uint64_t>
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
  public static explicit operator uint64_t(long v) => new uint64_t((ulong)v);

  public static implicit operator ulong (uint64_t v) => v.value;

  public override string ToString() => value.ToString();

  public bool Equals(uint64_t other) => value == other.value;

  public int CompareTo(uint64_t other) => value.CompareTo(other.value);
}

public struct int64_t : IEquatable<int64_t>, IComparable<int64_t>
{
  internal long value;

  public int64_t(long v)
  {
    value = v;
  }

  public static explicit operator int64_t(string s)
  {
    long value;

    if (s.StartsWith("0x"))
      value = long.Parse(s.Substring(2), System.Globalization.NumberStyles.HexNumber);
    else
      value = long.Parse(s);

    return new int64_t(value);
  }

  public static explicit operator int64_t(ulong v) => new int64_t((long)v);
  public static explicit operator int64_t(int v) => new int64_t((long)v);
  public static explicit operator int64_t(long v) => new int64_t((long)v);

  public static implicit operator long (int64_t v) => v.value;

  public override string ToString() => value.ToString();

  public bool Equals(int64_t other) => value == other.value;

  public int CompareTo(int64_t other) => value.CompareTo(other.value);
}

public class SubStateData
{
  public List<Ref<StateId, State>> states;
  public List<Ref<uint64_t, Operation>> operations;
  public List<ProfileData> profileData;
}


public struct ValueCount<T>
{
  public T value;
  public uint64_t count;
}

public class ExactValueData<T>
{
  public uint64_t count;
  public TransitionData data;
  public List<ValueCount<T>> values;
}

public class ExactValueDataWithAverage<T> : ExactValueData<T>
{
  public double average;
  public T min, max;
}

public class Analysis
{
  public string productName;
  public uint64_t majorVersion, minorVersion;
  public List<Ref<uint64_t, SubStateData>> subSystems;
  public List<Ref<uint64_t, ExactValueDataWithAverage<uint64_t>>> observedU64;
  public List<Ref<uint64_t, ExactValueDataWithAverage<int64_t>>> observedI64;
  public List<Ref<uint64_t, ExactValueData<string>>> observedString;

  public string GetLink(uint64_t subSystem, uint64_t operationIndex) => $"/op?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={subSystem}&id={operationIndex}";

  public string GetLink(uint64_t subSystem, StateId stateId) => $"/state?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={subSystem}&id={stateId.state}&sid={stateId.subState}";

  public HElement GetElementNameX(Info info, uint64_t subSystem, uint64_t operationIndex, string _class = null)
  {
    string name = info.GetOperationName(subSystem, operationIndex);

    return new HLink(name, GetLink(subSystem, operationIndex)) { Class = _class ?? "OperationLink", Title = name };
  }

  public HElement GetElementNameX(Info info, uint64_t subSystem, StateId stateId, string _class = null)
  {
    string name = info.GetStateName(subSystem, stateId.state, stateId.subState);

    return new HLink(name, GetLink(subSystem, stateId)) { Class = _class ?? "StateLink", Title = name };
  }

  public HElement GetElementBlob(Info info, uint64_t subSystem, uint64_t operationIndex, string _class = null) => new HLink("", GetLink(subSystem, operationIndex)) { Title = info.GetOperationName(subSystem, operationIndex), Class = _class == null ? "ElementBlobLink" : $"ElementBlobLink {_class}" };

  public HElement GetElementName(Info info, uint64_t subSystem, object value)
  {
    if (value is uint64_t)
      return GetElementNameX(info, subSystem, (uint64_t)value);
    else if (value is StateId)
      return GetElementNameX(info, subSystem, (StateId)value);
    else
      throw new Exception();
  }

  public HContainer ToPieChart<T>(uint64_t subSystem, List<Ref<T, TransitionData>> list, string name, Info info)
  {
    if (list.Count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    HContainer pieChart = new HContainer() { Class = "PieChartContainer" };
    HContainer description = new HContainer() { Class = "PieChartDescription" };

    ulong total = 0;

    foreach (var x in list)
      total += x.value.count;

    double offset = 0;
    int index = 0;
    string[] colors = { "#fff378", "#ffd070", "#ffaf7c", "#ff9293", "#fd80ac", "#d279c0", "#9777c9", "#4c75c2" };

    foreach (var x in list)
    {
      double percentage = ((double)x.value.count.value / (double)total) * 100.0;
      string color = colors[index++ % colors.Length];

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { GetElementName(info, subSystem, x.index), new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style= $"color:{color};" }, new HText($"{x.value.count}") { Class = "DataCount" }, new HText($"{x.value.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{x.value.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{x.value.maxDelay:0.####}s") { Class = "DataDelayMax" } } });

      offset += percentage;
    }

    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name), pieChart, description } };
  }

  public HContainer ToPieChart(List<Ref<uint64_t, uint64_t>> list, string name)
  {
    if (list.Count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    HContainer pieChart = new HContainer() { Class = "PieChartContainer" };
    HContainer description = new HContainer() { Class = "PieChartDescription" };

    ulong total = 0;

    foreach (var x in list)
      total += x.value;

    double offset = 0;
    int index = 0;
    string[] colors = { "#fff378", "#ffd070", "#ffaf7c", "#ff9293", "#fd80ac", "#d279c0", "#9777c9", "#4c75c2" };

    foreach (var x in list)
    {
      double percentage = ((double)x.value / (double)total) * 100.0;
      string color = colors[index++ % colors.Length];

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { new HText(x.index.ToString()) { Class = "DataName" }, new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style= $"color:{color};" }, new HText($"{x.value}") { Class = "DataCount" } } });

      offset += percentage;
    }

    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name), pieChart, description } };
  }

  public HContainer ToBarGraph<T>(uint64_t subSystem, List<Ref<T, TransitionData>> list, string name, Info info)
  {
    if (list.Count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    List<List<HElement>> contents = new List<List<HElement>>();

    ulong max = list[0].value.count;

    foreach (var x in list)
    {
      double percentage = ((double)x.value.count.value / (double)max) * 100.0;

      contents.Add(new List<HElement>() { new HText($"{x.value.count}") { Class = "Bar", Style = $"--value:{percentage};", Title = x.value.count.ToString() }, GetElementName(info, subSystem, x.index), new HContainer() { Elements = { new HText($"{x.value.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{x.value.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{x.value.maxDelay:0.####}s") { Class = "DataDelayMax" } } } });
    }

    HTable graph = new HTable(contents.ToArray()) { Class = "BarGraphContainer" };

    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name), graph } };
  }

  public HContainer ToLineGraph(List<Ref<uint64_t, uint64_t>> list)
  {
    if (list.Count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText("No Data Available.") } };

    HContainer graph = new HContainer() { Class = "LineGraph" };

    ulong total = 0;

    foreach (var x in list)
      total += x.value;

    int index = 0;
    string[] colors = { "#fff378", "#ffd070", "#ffaf7c", "#ff9293", "#fd80ac", "#d279c0", "#9777c9", "#4c75c2" };

    foreach (var x in list)
    {
      double percentage = ((double)x.value / (double)total) * 100.0;
      string color = colors[index++ % colors.Length];

      graph.AddElement(new HContainer() { Class = "LineGraphLine", Style = $"--value:{percentage}; background:{color}", Title = $"{x.index} ({x.value})" });
    }

    return graph;
  }

  public HContainer ToPieChart<T>(uint64_t subSystem, List<Ref<T, OperationTransitionData>> list, string name, Info info)
  {
    if (list.Count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    HContainer pieChart = new HContainer() { Class = "PieChartContainer" };
    HContainer description = new HContainer() { Class = "PieChartDescription" };

    ulong total = 0;

    foreach (var x in list)
        total += x.value.count;

    double offset = 0;
    int index = 0;
    string[] colors = { "#fff378", "#ffd070", "#ffaf7c", "#ff9293", "#fd80ac", "#d279c0", "#9777c9", "#4c75c2" };

    foreach (var x in list)
    {
      double percentage = ((double)x.value.count.value / (double)total) * 100.0;
      string color = colors[index++ % colors.Length];

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { GetElementName(info, subSystem, x.index), new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"color:{color};" }, new HText($"{x.value.count}") { Class = "DataCount" }, new HText($"{x.value.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{x.value.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{x.value.maxDelay:0.####}s") { Class = "DataDelayMax" }, ToLineGraph(x.value.operations) } });

      offset += percentage;
    }

    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name), pieChart, description } };
  }

  internal HElement ToPieChart<T>(List<ValueCount<T>> list, string name, Info info, uint64_t count)
  {
    if (list.Count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    HContainer pieChart = new HContainer() { Class = "PieChartContainer" };
    HContainer description = new HContainer() { Class = "PieChartDescription" };

    double offset = 0;
    ulong offsetCount = 0;

    int index = 0;
    string[] colors = { "#fff378", "#ffd070", "#ffaf7c", "#ff9293", "#fd80ac", "#d279c0", "#9777c9", "#4c75c2" };

    foreach (var x in list)
    {
      double percentage = ((double)x.count / (double)count.value) * 100.0;
      string color = colors[index++ % colors.Length];

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { new HText(x.value.ToString()) { Class = "DataName" }, new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"color:{color};" }, new HText($"{x.count}") { Class = "DataCount" } } });

      offset += percentage;
      offsetCount += x.count.value;
    }

    if (offsetCount < count.value)
    {
      double percentage = 1 - offset;
      string color = "#777";

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { new HText("Other") { Class = "DataName" }, new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"color:{color};" }, new HText($"{count.value - offsetCount}") { Class = "DataCount" } } });
    }

    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name), pieChart, description } };
  }

    public HContainer DisplayInfo<T>(ExactValueDataWithAverage<T> data)
  {
    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline("General"), new HText($"{data.count}") { Class = "DataCount" }, new HText($"{data.average:0.####}") { Class = "DataDelay" }, new HText($"{data.min:0.####}") { Class = "DataDelayMin" }, new HText($"{data.max:0.####}") { Class = "DataDelayMax" } } };
  }

  public HContainer DisplayInfo(TransitionData data)
  {
    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline("Timing Info"), new HText($"{data.count}") { Class = "DataCount" }, new HText($"{data.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{data.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{data.maxDelay:0.####}s") { Class = "DataDelayMax" } } };
  }

  public HContainer DisplayInfo(TransitionDataWithDelay data)
  {
    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline("Info"), new HText($"{data.count}") { Class = "DataCount" }, new HText($"{data.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{data.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{data.maxDelay:0.####}s") { Class = "DataDelayMax" }, new HText($"{data.avgStartDelay:0.####}s") { Class = "StartDelay" } } };
  }

  public void Sort()
  {
    foreach (var x in subSystems)
    {
      foreach (var y in x.value.states)
      {
        y.value.nextState = y.value.nextState.OrderByDescending(a => a.value.count).ToList();
        y.value.previousState = y.value.previousState.OrderByDescending(a => a.value.count).ToList();
        y.value.previousOperation = y.value.previousOperation.OrderByDescending(a => a.value.count).ToList();
        y.value.operations = y.value.operations.OrderByDescending(a => a.value.count).ToList();
        y.value.operationReach = y.value.operationReach.OrderByDescending(a => a.value.count).ToList();
        y.value.stateReach = y.value.stateReach.OrderByDescending(a => a.value.count).ToList();
      }

      foreach (var y in x.value.operations)
      {
        y.value.parentState = y.value.parentState.OrderByDescending(a => a.value.count).ToList();
        y.value.lastOperation = y.value.lastOperation.OrderByDescending(a => a.value.count).ToList();
        y.value.nextOperation = y.value.nextOperation.OrderByDescending(a => a.value.count).ToList();
        y.value.nextState = y.value.nextState.OrderByDescending(a => a.value.count).ToList();
        y.value.operationIndexCount = y.value.operationIndexCount.OrderByDescending(a => a.value).ToList();
      }
    }
  }

  internal HElement ToHistorgramChart(uint64_t subSystem, List<ProfileData> list, string name, Info info)
  {
    if (list.Count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    List<List<HElement>> contents = new List<List<HElement>>();

    ulong index = 0;

    foreach (var x in list)
    {
      HContainer histogram = new HContainer() { Class = "Histogram" };
      List<HElement> data = new List<HElement>() { histogram };

      ulong i = index++;
      ulong max = 0;

      foreach (var y in x.histogram)
        if (y > max)
          max = y;

      int j = -1;

      foreach (var y in x.histogram)
      {
        j++;

        double percentage = (double)y / max * 100.0;
        histogram.AddElement(new HContainer() { Class = "HistogramElement", Title = j < ProfileData.HistogramSizes.Length ? $"< {ProfileData.HistogramSizes[j]:0.##}ms ({y})" : $"> {ProfileData.HistogramSizes[ProfileData.HistogramSizes.Length - 1]:0}ms ({y})", Style = $"--value:{percentage};" });
      }

      data.Add(new HText(info.GetProfilerDataName(subSystem, (uint64_t)i)) { Class = "HistogramDataName", Title = $"Count: {x.timeMs.count}" });

      var minMaxContainer = new HContainer();
      data.Add(minMaxContainer);

      minMaxContainer.AddElement(new HText($"{x.timeMs.value:0.####}ms") { Class = "DataDelay" });

      if (x.minLastOperation.HasValue)
        minMaxContainer.AddElement(GetElementBlob(info, subSystem, x.minLastOperation.Value, "Min"));

      minMaxContainer.AddElement(new HText($"{x.timeMs.min:0.####}ms") { Class = "DataDelayMin", Title = x.minInfo.ToString() });

      minMaxContainer.AddElement(new HText($"{x.timeMs.max:0.####}ms") { Class = "DataDelayMax", Title = x.maxInfo.ToString() });

      if (x.maxLastOperation.HasValue)
        minMaxContainer.AddElement(GetElementBlob(info, subSystem, x.maxLastOperation.Value, "Max"));

      contents.Add(data);
    }

    HTable graph = new HTable(contents.ToArray()) { Class = "BarGraphContainer" };

    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name), graph } };
  }
}

public class Ref<Index, Value>
{
  public Index index;
  public Value value;
}

public struct StateId : IEquatable<StateId>
{
  public uint64_t state, subState;

  public StateId(ulong state, ulong subState) { this.state = new uint64_t(state); this.subState = new uint64_t(subState); }

  public bool Equals(StateId other) => state == other.state && subState == other.subState;
}

public class TransitionData
{
  public double avgDelay, maxDelay, minDelay;
  public uint64_t count;
}

public class OperationTransitionData : TransitionData
{
  public List<Ref<uint64_t, uint64_t>> operations;
}

public class TransitionDataWithDelay : TransitionData
{
  public double avgStartDelay;
}

public struct AvgValue<T>
{
  public uint64_t count;
  public double value;
  public T min, max;
}

public struct HardwareInfoShort
{
  public string cpu;
  public uint cpuCores;
  public uint64_t freeRam, totalRam;
  public string gpu;
  public uint64_t freeVRam, dedicatedVRam, totalVRam;
  public string os;
  public uint64_t monitorCount, multiMonitorWidth, multiMonitorHeight;

  public override string ToString()
  {
    return $"CPU: {cpu} ({cpuCores} Cores)\nRAM: {(double)freeRam / (1024.0 * 1024.0 * 1024.0):0.#} / {(double)totalRam / (1024.0 * 1024.0 * 1024.0):0.#} GB available\nGPU: {gpu} ({(double)freeVRam / (1024.0 * 1024.0 * 1024.0):0.#} / {(double)totalVRam / (1024.0 * 1024.0 * 1024.0):0.#} GB available, {(double)dedicatedVRam / (1024.0 * 1024.0 * 1024.0):0.#} GB dedicated)\nOS: {os}\nMonitors: {monitorCount} ({multiMonitorWidth} x {multiMonitorHeight} total)";
  }
}

public class ProfileData
{
  // 1.01^(x^1.7)-1
  public static double[] HistogramSizes = { 0.01, 0.03285697058147385, 0.06652806072336404, 0.110750954546329, 0.1658987828997884, 0.2327704810349522, 0.3125338060030061, 0.4067245919737239, 0.5172768409819379, 0.646575833877717, 0.7975327335576197, 0.9736822038758584, 1.179306592328094, 1.4195920019549, 1.700823462599401, 2.030628623602832, 2.418282116414415, 2.875086176114238, 3.414847505261783, 4.054476015351149, 4.814738383900456, 5.721208834388007, 6.80547186160106, 8.10664768624367, 9.673332224582024, 11.56607089221216, 13.86052174911005, 16.65151117747014, 20.05824827622747, 24.2310475824663, 29.36002049995231, 35.68634326613121, 43.51690606970438, 53.24341121737348, 65.36734142191057 };

  public uint64_t[] histogram;
  public AvgValue<double> timeMs;
  public HardwareInfoShort minInfo, maxInfo;
  public uint64_t? minLastOperation, maxLastOperation;
}

public class State : TransitionDataWithDelay
{
  public List<Ref<StateId, TransitionData>> nextState;
  public List<Ref<StateId, TransitionData>> previousState;
  public List<Ref<uint64_t, OperationTransitionData>> operations;
  public List<Ref<uint64_t, TransitionData>> previousOperation;
  public List<Ref<StateId, TransitionData>> stateReach;
  public List<Ref<uint64_t, TransitionData>> operationReach;
  public List<ProfileData> profileData;
}

public class Operation : TransitionDataWithDelay
{
  public List<Ref<StateId, TransitionData>> parentState;
  public List<Ref<StateId, TransitionData>> nextState;
  public List<Ref<uint64_t, OperationTransitionData>> nextOperation;
  public List<Ref<uint64_t, TransitionData>> lastOperation;
  public List<Ref<uint64_t, uint64_t>> operationIndexCount;
}

public class InfoSubSystem
{
  public List<string> states;
  public List<string> operations;
  public List<string> profilerData;
}

public class Info
{
  public string productName;
  public uint64_t majorVersion;
  public uint64_t? minorVersion;

  public List<Ref<uint64_t, InfoSubSystem>> subSystems;

  public List<string> observedValueU64;
  public List<string> observedValueI64;
  public List<string> observedValueString;

  public List<string> observedValueRangeU64;
  public List<string> observedValueRangeI64;
  public List<string> observedValueRangeF64;

  public string GetName(uint64_t subSystem, object x)
  {
    if (x is uint64_t)
      return GetOperationName(subSystem, (uint64_t)x);
    else if (x is StateId)
      return GetStateName(subSystem, ((StateId)x).state, ((StateId)x).subState);
    else
      throw new ArrayTypeMismatchException();
  }

  public string GetStateName(uint64_t subSystem, uint64_t stateIndex, uint64_t subStateIndex)
  {
    if (subSystems != null)
    {
      foreach (var subSys in subSystems)
      {
        if (subSys.index == subSystem)
        {
          if ((ulong)subSys.value.states.Count > stateIndex)
            return $"{subSys.value.states[(int)stateIndex.value]} ({subStateIndex})";

          break;
        }
      }
    }
    
    return $"State #{stateIndex} ({subStateIndex})";
  }

  public string GetOperationName(uint64_t subSystem, uint64_t operationType)
  {
    if (subSystems != null)
    {
      foreach (var subSys in subSystems)
      {
        if (subSys.index == subSystem)
        {
          if ((ulong)subSys.value.operations.Count > operationType)
            return subSys.value.operations[(int)operationType.value];

          break;
        }
      }
    }
    
    return $"Operation #{operationType}";
  }

  public string GetProfilerDataName(uint64_t subSystem, uint64_t dataIndex)
  {
    if (subSystems != null)
    {
      foreach (var subSys in subSystems)
      {
        if (subSys.index == subSystem)
        {
          if ((ulong)subSys.value.profilerData.Count > dataIndex)
            return subSys.value.profilerData[(int)dataIndex.value];

          break;
        }
      }
    }
    
    return $"Profile Region #{dataIndex}";
  }

  public string GetValueNameU64(ulong index)
  {
    if (observedValueU64 != null && observedValueU64.Count > (int)index)
      return observedValueU64[(int)index];

    return $"Value #{index}";
  }

  public string GetValueNameI64(ulong index)
  {
    if (observedValueI64 != null && observedValueI64.Count > (int)index)
      return observedValueI64[(int)index];

    return $"Value #{index}";
  }

  public string GetValueNameString(ulong index)
  {
    if (observedValueString != null && observedValueString.Count > (int)index)
      return observedValueString[(int)index];

    return $"Value #{index}";
  }

  public string GetValueRangeNameU64(ulong index)
  {
    if (observedValueRangeU64 != null && observedValueRangeU64.Count > (int)index)
      return observedValueRangeU64[(int)index];

    return $"Value Range #{index}";
  }

  public string GetValueRangeNameI64(ulong index)
  {
    if (observedValueRangeI64 != null && observedValueRangeI64.Count > (int)index)
      return observedValueRangeI64[(int)index];

    return $"Value Range #{index}";
  }

  public string GetValueRangeNameF64(ulong index)
  {
    if (observedValueRangeF64 != null && observedValueRangeF64.Count > (int)index)
      return observedValueRangeF64[(int)index];

    return $"Value Range #{index}";
  }
}
