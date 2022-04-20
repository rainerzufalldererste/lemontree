using System;
using System.Collections.Generic;
using LamestWebserver.UI;
using LamestWebserver.Core;

public static class GraphGen
{
  public static string GetLink(Analysis analysis, uint64_t subSystem, uint64_t operationIndex) => $"/op?p={analysis.productName.EncodeUrl()}&V={analysis.majorVersion}&v={analysis.minorVersion}&ss={subSystem}&id={operationIndex}";

  public static string GetLink(Analysis analysis, uint64_t subSystem, StateId stateId) => $"/state?p={analysis.productName.EncodeUrl()}&V={analysis.majorVersion}&v={analysis.minorVersion}&ss={subSystem}&id={stateId.state}&sid={stateId.subState}";

  public static HElement GetElementNameX(Analysis analysis, Info info, uint64_t subSystem, uint64_t operationIndex, string _class = null)
  {
    string name = info.GetOperationName(subSystem, operationIndex);

    return new HLink(name, GetLink(analysis, subSystem, operationIndex)) { Class = _class ?? "OperationLink", Title = name };
  }

  public static HElement GetElementNameX(Analysis analysis, Info info, uint64_t subSystem, StateId stateId, string _class = null)
  {
    string name = info.GetStateName(subSystem, stateId.state, stateId.subState);

    return new HLink(name, GetLink(analysis, subSystem, stateId)) { Class = _class ?? "StateLink", Title = name };
  }

  public static HElement GetElementBlob(Analysis analysis, Info info, uint64_t subSystem, uint64_t operationIndex, string _class = null) => new HLink("", GetLink(analysis, subSystem, operationIndex)) { Title = info.GetOperationName(subSystem, operationIndex), Class = _class == null ? "ElementBlobLink" : $"ElementBlobLink {_class}" };

  public static HElement GetElementName(Analysis analysis, Info info, uint64_t subSystem, object value)
  {
    if (value is uint64_t)
      return GetElementNameX(analysis, info, subSystem, (uint64_t)value);
    else if (value is StateId)
      return GetElementNameX(analysis, info, subSystem, (StateId)value);
    else
      throw new Exception();
  }

  public static HContainer ToPieChart<T>(Analysis analysis, uint64_t subSystem, List<Ref<T, TransitionData>> list, string name, Info info)
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
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { GetElementName(analysis, info, subSystem, x.index), new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"color:{color};" }, new HText($"{x.value.count}") { Class = "DataCount" }, new HText($"{x.value.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{x.value.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{x.value.maxDelay:0.####}s") { Class = "DataDelayMax" } } });

      offset += percentage;
    }

    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name), pieChart, description } };
  }

  public static HContainer ToPieChart(List<Ref<uint64_t, uint64_t>> list, string name)
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
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { new HText(x.index.ToString()) { Class = "DataName" }, new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"color:{color};" }, new HText($"{x.value}") { Class = "DataCount" } } });

      offset += percentage;
    }

    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name), pieChart, description } };
  }

  public static HContainer ToBarGraph<T>(Analysis analysis, uint64_t subSystem, List<Ref<T, TransitionData>> list, string name, Info info)
  {
    if (list.Count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    List<List<HElement>> contents = new List<List<HElement>>();

    ulong max = list[0].value.count;

    foreach (var x in list)
    {
      double percentage = ((double)x.value.count.value / (double)max) * 100.0;

      contents.Add(new List<HElement>() { new HText($"{x.value.count}") { Class = "Bar", Style = $"--value:{percentage};", Title = x.value.count.ToString() }, GetElementName(analysis, info, subSystem, x.index), new HContainer() { Elements = { new HText($"{x.value.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{x.value.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{x.value.maxDelay:0.####}s") { Class = "DataDelayMax" } } } });
    }

    HTable graph = new HTable(contents.ToArray()) { Class = "BarGraphContainer" };

    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name), graph } };
  }

  public static HContainer ToLineGraph(List<Ref<uint64_t, uint64_t>> list)
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

  public static HContainer ToPieChart<T>(Analysis analysis, uint64_t subSystem, List<Ref<T, OperationTransitionData>> list, string name, Info info)
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
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { GetElementName(analysis, info, subSystem, x.index), new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"color:{color};" }, new HText($"{x.value.count}") { Class = "DataCount" }, new HText($"{x.value.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{x.value.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{x.value.maxDelay:0.####}s") { Class = "DataDelayMax" }, ToLineGraph(x.value.operations) } });

      offset += percentage;
    }

    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name), pieChart, description } };
  }

  internal static HElement ToPieChart<T>(List<ValueCount<T>> list, string name, Info info, uint64_t count)
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

  public static HContainer DisplayInfo<T>(GlobalExactValueDataWithAverage<T> data)
  {
    return new HContainer() { Elements = { new HText($"{data.count}") { Class = "DataCount" }, new HText($"{data.average:0.####}") { Class = "DataDelay" }, new HText($"{data.min:0.####}") { Class = "DataDelayMin" }, new HText($"{data.max:0.####}") { Class = "DataDelayMax" } } };
  }

  public static HContainer ToPieChart<T>(GlobalExactValueData<T> data, string name)
  {
    if (data.count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    HContainer pieChart = new HContainer() { Class = "PieChartContainer" };
    HContainer description = new HContainer() { Class = "PieChartDescription" };

    double offset = 0;
    ulong offsetCount = 0;

    int index = 0;
    string[] colors = { "#fff378", "#ffd070", "#ffaf7c", "#ff9293", "#fd80ac", "#d279c0", "#9777c9", "#4c75c2" };

    foreach (var x in data.values)
    {
      double percentage = ((double)x.count / (double)data.count.value) * 100.0;
      string color = colors[index++ % colors.Length];

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { new HText(x.value.ToString()) { Class = "DataName" }, new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"color:{color};" }, new HText($"{x.count}") { Class = "DataCount" } } });

      offset += percentage;
      offsetCount += x.count.value;
    }

    if (offsetCount < data.count.value)
    {
      double percentage = 1 - offset;
      string color = "#777";

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { new HText("Other") { Class = "DataName" }, new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"color:{color};" }, new HText($"{data.count.value - offsetCount}") { Class = "DataCount" } } });
    }

    var ret = new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name) } };

    if (data is GlobalExactValueDataWithAverage<T>)
      ret.AddElement(DisplayInfo((GlobalExactValueDataWithAverage<T>)data));

    ret.AddElement(pieChart);
    ret.AddElement(description);

    return ret;
  }

  public static HContainer DisplayInfo<T>(ExactValueDataWithAverage<T> data)
  {
    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline("General"), new HText($"{data.count}") { Class = "DataCount" }, new HText($"{data.average:0.####}") { Class = "DataDelay" }, new HText($"{data.min:0.####}") { Class = "DataDelayMin" }, new HText($"{data.max:0.####}") { Class = "DataDelayMax" } } };
  }

  public static HContainer DisplayInfo(TransitionData data)
  {
    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline("Timing Info"), new HText($"{data.count}") { Class = "DataCount" }, new HText($"{data.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{data.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{data.maxDelay:0.####}s") { Class = "DataDelayMax" } } };
  }

  public static HContainer DisplayInfo(TransitionDataWithDelay data)
  {
    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline("Info"), new HText($"{data.count}") { Class = "DataCount" }, new HText($"{data.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{data.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{data.maxDelay:0.####}s") { Class = "DataDelayMax" }, new HText($"{data.avgStartDelay:0.####}s") { Class = "StartDelay" } } };
  }

  internal static HElement ToHistorgramChart(Analysis analysis, uint64_t subSystem, List<ProfileData> list, string name, Info info)
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
        minMaxContainer.AddElement(GetElementBlob(analysis, info, subSystem, x.minLastOperation.Value, "Min"));

      minMaxContainer.AddElement(new HText($"{x.timeMs.min:0.####}ms") { Class = "DataDelayMin", Title = x.minInfo.ToString() });

      minMaxContainer.AddElement(new HText($"{x.timeMs.max:0.####}ms") { Class = "DataDelayMax", Title = x.maxInfo.ToString() });

      if (x.maxLastOperation.HasValue)
        minMaxContainer.AddElement(GetElementBlob(analysis, info, subSystem, x.maxLastOperation.Value, "Max"));

      contents.Add(data);
    }

    HTable graph = new HTable(contents.ToArray()) { Class = "BarGraphContainer" };

    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name), graph } };
  }

  public static HContainer DisplayInfo<T>(ValueRange<T> data)
  {
    return new HContainer() { Elements = { new HHeadline("Delay", 2), new HText($"{data.data.count}") { Class = "DataCount" }, new HText($"{data.data.avgDelay:0.####} s") { Class = "DataDelay" }, new HText($"{data.data.minDelay:0.####} s") { Class = "DataDelayMin" }, new HText($"{data.data.maxDelay:0.####} s") { Class = "DataDelayMax" } } };
  }

  internal static HElement ToHistorgramChart<T>(GlobalValueRange<T> data, string name)
  {
    if (data.count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    var ret = new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name) } };

    if (data is PerfValueRange<T>)
    {
      var d = data as PerfValueRange<T>;

      ret.AddElement(new HContainer() { Elements = { new HText($"{data.count}") { Class = "DataCount" }, new HText($"{data.average:0.####}") { Class = "DataDelay" }, new HText($"{data.min:0.####}") { Class = "DataDelayMin", Title = d.minInfo.ToString() }, new HText($"{data.max:0.####}") { Class = "DataDelayMax", Title = d.maxInfo.ToString() } } });
    }
    else
    {
      ret.AddElement(new HContainer() { Elements = { new HText($"{data.count}") { Class = "DataCount" }, new HText($"{data.average:0.####}") { Class = "DataDelay" }, new HText($"{data.min:0.####}") { Class = "DataDelayMin" }, new HText($"{data.max:0.####}") { Class = "DataDelayMax" } } });
    }

    if (data is ValueRange<T>)
      ret.AddElement(DisplayInfo((ValueRange<T>)data));

    if (data.histogram.Length > 1)
    {
      HContainer histogram = new HContainer() { Class = "Histogram Large" };

      ulong max = 0;

      double[] histVal = new double[data.histogram.Length];

      int i = 0;
      double min = data.min.ToDouble();
      double diff = (double)(data.max.ToDouble() - min);

      foreach (var y in data.histogram)
      {
        if (y > max)
          max = y;

        histVal[i] = min + diff * ((double)i / (double)(data.histogram.Length - 1));
        i++;
      }

      i = -1;

      foreach (var y in data.histogram)
      {
        i++;

        double percentage = (double)y / max * 100.0;
        histogram.AddElement(new HContainer() { Class = "HistogramElement Large", Title = $"{histVal[i]:0.###} ({y})", Style = $"--value:{percentage};" });
      }

      ret.AddElement(histogram);

      ret.AddElement(new HText(data.min.ToString()) { Class = "HistMin" });
      ret.AddElement(new HText(data.max.ToString()) { Class = "HistMax" });
    }

    return ret;
  }

  internal static HElement ToHourHistorgramChart(List<uint64_t> data, string name)
  {
    if (data.Count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    var ret = new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name) } };

    HContainer histogram = new HContainer() { Class = "Histogram Large" };

    uint64_t max = (uint64_t)0;

    foreach (var y in data)
      if (y > max)
        max = y;

    int i = 0;

    foreach (var y in data)
    {
      double percentage = (double)y / max * 100.0;
      histogram.AddElement(new HContainer() { Class = "HistogramElement Large", Title = $"{i}:00 ({y})", Style = $"--value:{percentage};" });
      i++;
    }

    ret.AddElement(histogram);

    return ret;
  }

  internal static HElement ToDayHistorgramChart(List<uint64_t> data, uint64_t firstDayTimestamp, string name)
  {
    if (data.Count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    var ret = new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name) } };

    DateTime start = DateTime.FromFileTimeUtc((long)firstDayTimestamp.value);
    int preceedingDays = 0;

    while (start.DayOfWeek != DayOfWeek.Sunday)
    {
      start -= TimeSpan.FromDays(1);
      preceedingDays++;
    }

    List<List<HElement>> table = new List<List<HElement>>() { new List<HElement>(), new List<HElement>(), new List<HElement>(), new List<HElement>(), new List<HElement>(), new List<HElement>(), new List<HElement>() };

    uint64_t max = (uint64_t)0;

    foreach (var x in data)
      if (x.value > max.value)
        max = x;

    for (int i = 0; i < data.Count; i++)
    {
      double percentage = ((double)data[i] / (double)max) * 100.0;

      TimeSpan offset = TimeSpan.FromDays(i + preceedingDays);
      DateTime day = start + offset;

      while ((int)offset.TotalDays / 7 >= table[(int)day.DayOfWeek].Count)
        table[(int)day.DayOfWeek].Add(new HMultipleElements());

      table[(int)day.DayOfWeek][(int)offset.TotalDays / 7] = new HText() { Class = "DayHistElement", Style = $"--value:{percentage}", Title = day.ToString("ddd dd. MMMM yyyy") + $" ({data[i]})" };
    }

    ret.AddElement(new HTable(table) { Class = "DayHistogram" });

    return ret;
  }

  private static void AppendErrorData(HContainer container, ulong maxCount, Error e, TransitionData t)
  {
    double percentage = ((double)t.count.value / (double)maxCount) * 100.0;

    var errorInfo = new HContainer() { ID = Hash.GetHash(), Class = "ErrorInfo" };

    container.AddElement(new HContainer() { Class = "ErrorDescription", Elements = { new HContainer() { Class = "ErrorBarContainer", Elements = { new HText($"{t.count}") { Class = "BarError", Style = $"--value:{percentage};", Title = t.count.ToString() } } }, new HText(e.errorCode.ToString()) { Class = "ErrorCode", Name = e.description ?? "" }, new HButton("+", "", $"document.getElementById(\"{errorInfo.ID}\").style.display = \"block\";") { Class = "ErrorInfoShowButton" }, new HContainer() { Elements = { new HText($"{t.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{t.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{t.maxDelay:0.####}s") { Class = "DataDelayMax" } } } } });

    container.AddElement(errorInfo);

    if (e is Crash)
      errorInfo.AddElement(new HText((e as Crash).firstOccurence) { Class = "CrashFirstOccurence" });

    if (!string.IsNullOrWhiteSpace(e.description))
      errorInfo.AddElement(new HText(e.description) { Class = "ErrorDescriptionText" });

    if (e.stackTrace != null)
    {
      var stackTrace = new HContainer() { Class = "StackTrace" };
      errorInfo.AddElement(stackTrace);

      foreach (var s in e.stackTrace)
      {
        var element = new HContainer() { Class = "StackTraceElement" };
        stackTrace.Elements.Add(element);

        if (!string.IsNullOrWhiteSpace(s.module))
          element.AddElement(new HText(s.module) { Class = "StackTraceElementModule" });

        if (!string.IsNullOrWhiteSpace(s.function))
          element.AddElement(new HText(s.function) { Class = "StackTraceElementFunctionName" });

        if (!string.IsNullOrWhiteSpace(s.file))
        {
          element.AddElement(new HText(s.file) { Class = "StackTraceElementFile" });

          if (s.line.HasValue)
            element.AddElement(new HText(s.line.Value.ToString()) { Class = "StackTraceElementLine" });
        }

        if (element.Elements.Count == 0)
          element.AddElement(new HText() { Class = "StackTraceElementAppModule" });

        element.AddElement(new HText($"0x{s.offset:X}") { Class = "StackTraceElementOffset" });
      }
    }

    if (errorInfo.Elements.Count == 0)
      errorInfo.AddElement(new HText("No Extended Error Information Available."));
  }

  internal static HElement ToErrorChart(List<ErrorData> data, string name)
  {
    if (data.Count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    var ret = new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name) } };

    ulong max = 0;

    foreach (var x in data)
      if (max < x.data.count)
        max = x.data.count;

    foreach (var x in data)
      AppendErrorData(ret, max, x.error, x.data);

    return ret;
  }

  internal static HElement ToErrorChart(Analysis analysis, uint64_t subSystem, List<Ref<ErrorId, TransitionData>> data, string name, bool isErrorNotWarningsList)
  {
    if (data.Count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    var ret = new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name) } };

    SubSystemData subSys = analysis.subSystems.FindItem(subSystem);
    List<Tuple<Error, TransitionData>> errors = new List<Tuple<Error, TransitionData>>();
    ulong max = 0;

    foreach (var x in data)
    {
      if (max < x.value.count)
        max = x.value.count;

      Error e;

      if (x.index.state.HasValue)
      {
        if (isErrorNotWarningsList)
          e = subSys.states.FindItem(x.index.state.Value).errors[(int)x.index.errorIndex.value].error;
        else
          e = subSys.states.FindItem(x.index.state.Value).warnings[(int)x.index.errorIndex.value].error;
      }
      else
      {
        if (isErrorNotWarningsList)
          e = subSys.noStateErrors[(int)x.index.errorIndex.value].error;
        else
          e = subSys.noStateWarnings[(int)x.index.errorIndex.value].error;
      }

      errors.Add(Tuple.Create(e, x.value));
    }

    foreach (var x in errors)
      AppendErrorData(ret, max, x.Item1, x.Item2);

    return ret;

  }

  internal static HElement ToErrorChart(List<CrashData> data, string name)
  {
    if (data.Count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    var ret = new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name) } };

    ulong max = 0;

    foreach (var x in data)
      if (max < x.data.count)
        max = x.data.count;

    foreach (var x in data)
      AppendErrorData(ret, max, x.crash, x.data);

    return ret;
  }
}
