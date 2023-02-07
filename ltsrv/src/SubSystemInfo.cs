using System.Collections.Generic;
using System.Linq;
using LamestWebserver;
using LamestWebserver.UI;
using LamestWebserver.Core;

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
      return ltsrv.GetPage("Major Versions", ShowMajorVersions(sessionData, productName));
    else if (!ulong.TryParse(sessionData.HttpHeadVariables["v"], out minorVersion))
      return ltsrv.GetPage("Minor Versions", ShowMinorVersions(sessionData, productName, majorVersion));
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
    yield return new HLink("Products", "/subsystem") { Class = "nav" };

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
    yield return new HLink("Products", "/subsystem") { Class = "nav" };
    yield return new HLink(productName, $"/subsystem?p={productName.EncodeUrl()}") { Class = "nav" };

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
    yield return new HLink("Products", "/subsystem") { Class = "nav" };
    yield return new HLink(productName, $"/subsystem?p={productName.EncodeUrl()}") { Class = "nav" };
    yield return new HLink(majorVersion.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}") { Class = "nav" };

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline($"'{productName}' (Version {majorVersion}.{minorVersion})");

    yield return GraphGen.ToDayHistorgramChart(analysis.days, analysis.firstDayTimestamp, "Days (UTC)");

    yield return GraphGen.ToHourHistorgramChart(analysis.hours, "Hours (UTC)");

    yield return new HHeadline($"SubSystems", 2);

    foreach (var x in analysis.subSystems)
      yield return new HLink(info.GetSubSystemName(x.index), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={x.index}") { Class = "LargeButton" };

    yield return new HHeadline($"Crashes", 2);

    yield return GraphGen.ToErrorChart(analysis.crashes, "Crashes");

    yield return new HHeadline($"Observed Values UInt64", 2);

    if (analysis.observedU64 != null)
      foreach (var x in analysis.observedU64)
        yield return new HLink(info.GetValueNameU64(x.index.value), $"/value?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=u64&id={x.index}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Values Int64", 2);

    if (analysis.observedI64 != null)
      foreach (var x in analysis.observedI64)
        yield return new HLink(info.GetValueNameI64(x.index.value), $"/value?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=i64&id={x.index}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Values String", 2);

    if (analysis.observedString != null)
      foreach (var x in analysis.observedString)
        yield return new HLink(info.GetValueNameString(x.index.value), $"/value?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=string&id={x.index}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Value Range UInt64", 2);

    if (analysis.observedRangeU64 != null)
      foreach (var x in analysis.observedRangeU64)
        yield return new HLink(info.GetValueRangeNameU64(x.index.value), $"/range?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=u64&id={x.index}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Value Range Int64", 2);

    if (analysis.observedRangeI64 != null)
      foreach (var x in analysis.observedRangeI64)
        yield return new HLink(info.GetValueRangeNameI64(x.index.value), $"/range?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=i64&id={x.index}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Value Range Float64", 2);

    if (analysis.observedRangeF64 != null)
      foreach (var x in analysis.observedRangeF64)
        yield return new HLink(info.GetValueRangeNameF64(x.index.value), $"/range?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=f64&id={x.index}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Value Range Vector2<Float32>", 2);

    if (analysis.observedRangeF32_2 != null)
      foreach (var x in analysis.observedRangeF32_2)
        yield return new HLink(info.GetValueRangeNameF32_2(x.index.value), $"/range?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=f32_2&id={x.index}") { Class = "LargeButton" };

    yield return new HHeadline($"Performance Metrics", 2);

    if (analysis.perfMetrics != null)
      foreach (var x in analysis.perfMetrics)
        yield return new HLink(info.GetPerfMetricName(x.index.value), $"/perfmet?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&id={x.index}") { Class = "LargeButton" };

    yield return new HHeadline($"Hardware Info", 2);

    yield return GraphGen.ToPieChart(analysis.hwInfo.cpu, "CPU");
    yield return GraphGen.ToPieChart(analysis.hwInfo.cpuCores, "CPU Cores");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.totalPhysicalRam, "Total Physical RAM GB");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.availablePhysicalRam, "Available Physical RAM GB");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.totalVirtualRam, "Total Virtual RAM GB");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.availableVirtualRam, "Available Virtual RAM GB");
    yield return GraphGen.ToPieChart(analysis.hwInfo.os, "Operating System");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.gpuDedicatedVRam, "GPU Dedicated VRAM GB");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.gpuSharedVRam, "GPU Shared VRAM GB");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.gpuTotalVRam, "GPU Total VRAM GB");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.gpuFreeVRam, "GPU Free VRAM GB");
    yield return GraphGen.ToPieChart(analysis.hwInfo.gpu, "GPU");
    yield return GraphGen.ToPieChart(analysis.hwInfo.gpuVendorId, "GPU Vendor");
    yield return GraphGen.ToPieChart(analysis.hwInfo.primaryLanguage, "Primary UI Language");
    yield return GraphGen.ToPieChart(analysis.hwInfo.isElevated, "isElevated");
    yield return GraphGen.ToPieChart(analysis.hwInfo.monitorCount, "Monitor Count");
    yield return GraphGen.ToPieChart(analysis.hwInfo.monitorSize, "Monitor Size");
    yield return GraphGen.ToPieChart(analysis.hwInfo.totalMonitorSize, "Total Monitor Size");
    yield return GraphGen.ToPieChart(analysis.hwInfo.monitorDpi, "Monitor DPI");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.availableStorage, "Available Storage GB");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.totalStorage, "Total Storage GB");
    yield return GraphGen.ToPieChart(analysis.hwInfo.deviceManufacturer, "Device Manufacturer");
    yield return GraphGen.ToPieChart(analysis.hwInfo.deviceManufacturerModel, "Device Model");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.totalSsdStorage, "Total SSD Storage GB");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.ssdStorageShare, "SSD Storage Share", v => $"{v * 100.0:0.##}%");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.downLinkSpeed, "Network Down Link", v => $"{v / 1000.0:0.##} GBit/s");
    yield return GraphGen.ToHistorgramChart(analysis.hwInfo.upLinkSpeed, "Network Up Link", v => $"{v / 1000.0:0.##} GBit/s");
    yield return GraphGen.ToPieChart(analysis.hwInfo.isWireless, "Network Connection", v => v ? "IEEE 802.11" : "Ethernet");
    yield return GraphGen.ToPieChart(analysis.hwInfo.identifier, "Network ID", v => HardwareInfo.idToString(v));
  }

  internal static IEnumerable<HElement> ShowContents(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong subSystem)
  {
    yield return new HLink("Products", "/subsystem") { Class = "nav" };
    yield return new HLink(productName, $"/subsystem?p={productName.EncodeUrl()}") { Class = "nav" };
    yield return new HLink(majorVersion.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}") { Class = "nav" };
    yield return new HLink(minorVersion.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}") { Class = "nav" };

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline($"SubSystem {subSystem} of {productName} (Version {majorVersion}.{minorVersion})");

    yield return new HHeadline("States", 2);

    var s = analysis.subSystems.FindItem((uint64_t)subSystem);

    foreach (var x in s.states)
      yield return new HLink(info.GetStateName((uint64_t)subSystem, x.index.state, x.index.subState, (x.value.errors.Count != 0 ? "🚫" : "") + (x.value.warnings.Count != 0 ? "⚠️" : "")), $"/state?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={subSystem}&id={x.index.state}&sid={x.index.subState}") { Class = "Button" };

    yield return new HHeadline("Operations", 2);

    foreach (var x in s.operations)
      yield return new HLink(info.GetOperationName((uint64_t)subSystem, x.index, (x.value.errors.Count != 0 ? "🚫" : "") + (x.value.warnings.Count != 0 ? "⚠️" : "")), $"/op?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={subSystem}&id={x.index}") { Class = "Button" };

    yield return GraphGen.ToErrorChart(s.noStateErrors, "Errors not attributable to a state");
    yield return GraphGen.ToErrorChart(s.noStateWarnings, "Warnings not attributable to a state");
    yield return GraphGen.ToPieChart(s.noStateLogs.values, "Log Messages not attributed to a state", info, s.noStateLogs.count);

    yield return GraphGen.ToHistorgramChart(analysis, (uint64_t)subSystem, s.profileData, "Performance", info);

    // Display States with relevance information.
    if (s.states != null)
    {
      yield return new HHeadline("States by time / ops", 2);

      bool first = true;
      double max = 0;

      double maxOps = s.states.Max(e => e.value.operations.Sum(f => (double)(ulong)f.value.count));

      foreach (var x in s.states.OrderByDescending(e => e.value.avgDelay))
      {
        if (first)
        {
          first = false;
          max = x.value.avgDelay;
        }

        var opCount = x.value.operations.Sum(e => (double)(ulong)e.value.count);

        yield return new HLink(info.GetStateName((uint64_t)subSystem, x.index.state, x.index.subState, (x.value.errors.Count != 0 ? "🚫" : "") + (x.value.warnings.Count != 0 ? "⚠️" : "")), $"/state?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={subSystem}&id={x.index.state}&sid={x.index.subState}") { Class = "SortedByWeight", Style = $"--w:{x.value.avgDelay / max * 100};--s:{opCount / maxOps * 100}", Title = info.GetStateName((uint64_t)subSystem, x.index.state, x.index.subState, (x.value.errors.Count != 0 ? "🚫" : "") + (x.value.warnings.Count != 0 ? "⚠️" : "")), Elements = { new HText($"{x.value.avgDelay:0.##} s | {opCount} Operations") } };
      }
    }
  }
}
