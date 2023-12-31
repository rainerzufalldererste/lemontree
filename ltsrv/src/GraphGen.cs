﻿using System;
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
    string[] colors = { "0.0", "0.125", "0.25", "0.375", "0.5", "0.652", "0.75", "0.875", "1.0" };

    foreach (var x in list)
    {
      double percentage = ((double)x.value.count.value / (double)total) * 100.0;
      string color = colors[index++ % colors.Length];

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { GetElementName(analysis, info, subSystem, x.index), new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"--col: {color};" }, new HText($"{x.value.count}") { Class = "DataCount" }, new HText($"{x.value.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{x.value.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{x.value.maxDelay:0.####}s") { Class = "DataDelayMax" } } });

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
    string[] colors = { "0.0", "0.125", "0.25", "0.375", "0.5", "0.652", "0.75", "0.875", "1.0" };

    foreach (var x in list)
    {
      double percentage = ((double)x.value / (double)total) * 100.0;
      string color = colors[index++ % colors.Length];

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { new HText(x.index.ToString()) { Class = "DataName" }, new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"--col: {color};" }, new HText($"{x.value}") { Class = "DataCount" } } });

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
    string[] colors = { "0.0", "0.125", "0.25", "0.375", "0.5", "0.652", "0.75", "0.875", "1.0" };

    foreach (var x in list)
    {
      double percentage = ((double)x.value / (double)total) * 100.0;
      string color = colors[index++ % colors.Length];

      graph.AddElement(new HContainer() { Class = "LineGraphLine", Style = $"--value:{percentage}; --col: {color}", Title = $"{x.index} ({x.value})" });
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
    string[] colors = { "0.0", "0.125", "0.25", "0.375", "0.5", "0.652", "0.75", "0.875", "1.0" };

    foreach (var x in list)
    {
      double percentage = ((double)x.value.count.value / (double)total) * 100.0;
      string color = colors[index++ % colors.Length];

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { GetElementName(analysis, info, subSystem, x.index), new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"--col: {color};" }, new HText($"{x.value.count}") { Class = "DataCount" }, new HText($"{x.value.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{x.value.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{x.value.maxDelay:0.####}s") { Class = "DataDelayMax" }, ToLineGraph(x.value.operations) } });

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
    string[] colors = { "0.0", "0.125", "0.25", "0.375", "0.5", "0.652", "0.75", "0.875", "1.0" };

    foreach (var x in list)
    {
      double percentage = ((double)x.count / (double)count.value) * 100.0;
      string color = colors[index++ % colors.Length];

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { new HText(x.value.ToString()) { Class = "DataName" }, new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"--col: {color};" }, new HText($"{x.count}") { Class = "DataCount" } } });

      offset += percentage;
      offsetCount += x.count.value;
    }

    if (offsetCount < count.value)
    {
      double percentage = 1 - offset;
      string color = "#777";

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { new HText("Other") { Class = "DataName" }, new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"--col: {color};" }, new HText($"{count.value - offsetCount}") { Class = "DataCount" } } });
    }

    return new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name), pieChart, description } };
  }

  public static HContainer DisplayInfo<T>(GlobalExactValueDataWithAverage<T> data)
  {
    return new HContainer() { Elements = { new HText($"{data.count}") { Class = "DataCount" }, new HText($"{data.average:0.####}") { Class = "DataDelay" }, new HText($"{data.min:0.####}") { Class = "DataDelayMin" }, new HText($"{data.max:0.####}") { Class = "DataDelayMax" } } };
  }

  public static HContainer ToPieChart<T>(GlobalExactValueData<T> data, string name, Func<T, string> toStringFunc = null)
  {
    if (data == null || data.count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    HContainer pieChart = new HContainer() { Class = "PieChartContainer" };
    HContainer description = new HContainer() { Class = "PieChartDescription" };

    double offset = 0;
    ulong offsetCount = 0;

    int index = 0;
    string[] colors = { "0.0", "0.125", "0.25", "0.375", "0.5", "0.652", "0.75", "0.875", "1.0" };

    foreach (var x in data.values)
    {
      double percentage = ((double)x.count / (double)data.count.value) * 100.0;
      string color = colors[index++ % colors.Length];

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { new HText(toStringFunc == null ? x.value.ToString() : toStringFunc(x.value)) { Class = "DataName" }, new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"--col: {color};" }, new HText($"{x.count}") { Class = "DataCount" } } });

      offset += percentage;
      offsetCount += x.count.value;
    }

    if (offsetCount < data.count.value)
    {
      double percentage = 1 - offset;
      string color = "#777";

      pieChart.AddElement(new HContainer() { Class = "PieSegment", Style = $"--offset: {offset}; --value: {percentage}; --bg: {color};" + (percentage > 50 ? " --over50: 1;" : "") });
      description.AddElement(new HContainer() { Class = "PieDescriptionContainer", Elements = { new HText("Other") { Class = "DataName" }, new HText($"{percentage:0.##}%") { Class = "DataPercentage", Style = $"--col: {color};" }, new HText($"{data.count.value - offsetCount}") { Class = "DataCount" } } });
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

  internal static HElement ToHistorgramChart<T>(GlobalValueRange<T> data, string name, Func<double, string> toStringFunc = null)
  {
    if (data == null || data.count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    var ret = new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name) } };

    if (data is PerfValueRange<T>)
    {
      var d = data as PerfValueRange<T>;

      ret.AddElement(new HContainer() { Elements = { new HText($"{data.count}") { Class = "DataCount" }, new HText(toStringFunc == null ? $"{data.average:0.####}" : toStringFunc(data.average)) { Class = "DataDelay" }, new HText(toStringFunc == null ? $"{data.min:0.####}" : toStringFunc(Convert.ToDouble(data.min))) { Class = "DataDelayMin", Title = d.minInfo.ToString() }, new HText(toStringFunc == null ? $"{data.max:0.####}" : toStringFunc(Convert.ToDouble(data.max))) { Class = "DataDelayMax", Title = d.maxInfo.ToString() } } });
    }
    else
    {
      ret.AddElement(new HContainer() { Elements = { new HText($"{data.count}") { Class = "DataCount" }, new HText(toStringFunc == null ? $"{data.average:0.####}" : toStringFunc(data.average)) { Class = "DataDelay" }, new HText(toStringFunc == null ? $"{data.min:0.####}" : toStringFunc(Convert.ToDouble(data.min))) { Class = "DataDelayMin" }, new HText(toStringFunc == null ? $"{data.max:0.####}" : toStringFunc(Convert.ToDouble(data.max))) { Class = "DataDelayMax" } } });
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
        histogram.AddElement(new HContainer() { Class = "HistogramElement Large", Title = (toStringFunc == null ? $"{histVal[i]:0.###}" : toStringFunc(histVal[i])) + $"({y})", Style = $"--value:{percentage};" });
      }

      ret.AddElement(histogram);

      ret.AddElement(new HText(toStringFunc == null ? data.min.ToString() : toStringFunc(Convert.ToDouble(data.min))) { Class = "HistMin" });
      ret.AddElement(new HText(toStringFunc == null ? data.max.ToString() : toStringFunc(Convert.ToDouble(data.max))) { Class = "HistMax" });
    }

    return ret;
  }

  internal static HElement To2DHistorgramChart(ValueRange2D data, string name)
  {
    if (data.count == 0)
      return new HContainer() { Class = "NoData", Elements = { new HText($"No '{name}' Data Available.") } };

    var ret = new HContainer() { Class = "DataInfo", Elements = { new HHeadline(name) } };

    ret.AddElement(new HContainer() { Elements = { 
        new HText($"{data.count}") { Class = "DataCount" }, new HText($"{data.averageX:0.####}") { Class = "DataDelay 2D" }, new HText($"{data.averageY:0.####}") { Class = "DataDelay 2D" }, new HText($"{data.minX:0.####}") { Class = "DataDelayMin 2D" }, new HText($"{data.maxX:0.####}") { Class = "DataDelayMax 2D" } , new HText($"{data.minY:0.####}") { Class = "DataDelayMin 2D" }, new HText($"{data.maxY:0.####}") { Class = "DataDelayMax 2D" } 
      } });

    if (data.data != null)
      ret.AddElement(DisplayInfo(data.data));

    if (data.histogram.Length > 1)
    {
      ulong max = 0;
      ulong sum = 0;

      double[] histValX = new double[data.histogram.Length];
      double[] histValY = new double[data.histogram.Length];

      int i = 0;
      double minX = data.minX.ToDouble();
      double minY = data.minY.ToDouble();
      double diffX = (double)(data.maxX.ToDouble() - minX);
      double diffY = (double)(data.maxY.ToDouble() - minY);

      foreach (var x in data.histogram)
      {
        if (x == null || x.Length != histValX.Length)
        {
          ret.AddElement(new HText("Unsupported Data Format") { Class = "error" });
          return ret;
        }

        foreach (var y in x)
        {
          sum += y;

          if (y > max)
            max = y;
        }

        histValX[i] = minX + diffX * ((double)i / (double)(data.histogram.Length - 1));
        histValY[i] = minY + diffY * ((double)i / (double)(data.histogram.Length - 1));

        i++;
      }

      List<List<HElement>> hist2d = new List<List<HElement>>();

      for (int y = 0; y < data.histogram.Length; y++)
      {
        List<HElement> line = new List<HElement>();

        for (int x = 0; x < data.histogram[y].Length; x++)
        {
          double percentage = data.histogram[y][x] / (double)max * 100.0;
          line.Add(new HContainer() { Class = "Hist2DElement", Title = $"{histValX[x]:0.###}, {histValX[y]:0.###} ({x}, {y}) - {(data.histogram[y][x] / (double)sum) * 100.0:0.##} %", Style = $"--value:{percentage};" });
        }

        hist2d.Add(line);
      }

      ret.AddElement(new HTable(hist2d) { Class = "hist2d" });

      ret.AddElement(new HText(data.minX.ToString()) { Class = "HistMin 2D" });
      ret.AddElement(new HText(data.maxX.ToString()) { Class = "HistMax 2D" });
      ret.AddElement(new HText(data.minY.ToString()) { Class = "HistMin 2D" });
      ret.AddElement(new HText(data.maxY.ToString()) { Class = "HistMax 2D" });
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

  private static string ShowById(string ID, string displayType = "block") => $"document.getElementById(\"{ID}\").style.display = \"{displayType}\";";

  private static string HideById(string ID) => $"document.getElementById(\"{ID}\").style.display = \"none\";";

  private static void AppendErrorData(HContainer container, ulong maxCount, Error e, TransitionData t)
  {
    double percentage = ((double)t.count.value / (double)maxCount) * 100.0;

    var errorInfo = new HContainer() { ID = Hash.GetHash(), Class = "ErrorInfo" };
    
    string expandId = Hash.GetHash();
    string collapseId = Hash.GetHash();

    var expandErrorInfoButton = new HButton("+", "", ShowById(errorInfo.ID) + ShowById(collapseId, "inline") + HideById(expandId)) { Class = "ErrorInfoShowButton", ID = expandId };
    var collapseErrorInfoButton = new HButton("-", "", HideById(errorInfo.ID) + HideById(collapseId) + ShowById(expandId, "inline")) { Class = "ErrorInfoHideButton", ID = collapseId };

    container.AddElement(new HContainer() { Class = "ErrorDescription", Elements = { new HContainer() { Class = "ErrorBarContainer", Elements = { new HText($"{t.count}") { Class = "BarError", Style = $"--value:{percentage};", Title = t.count.ToString() } } }, new HText($"{e.errorCode} / 0x{e.errorCode.value:X}") { Class = "ErrorCode", Name = e.description ?? "" }, expandErrorInfoButton, collapseErrorInfoButton, new HContainer() { Elements = { new HText($"{t.avgDelay:0.####}s") { Class = "DataDelay" }, new HText($"{t.minDelay:0.####}s") { Class = "DataDelayMin" }, new HText($"{t.maxDelay:0.####}s") { Class = "DataDelayMax" } } } } });

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

        if (!string.IsNullOrWhiteSpace(s.disassembly))
          element.AddElement(new HText(s.disassembly) { Class = "StackTraceDisasm" });
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
