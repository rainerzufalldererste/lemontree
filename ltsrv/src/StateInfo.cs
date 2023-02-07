using System;
using System.Collections.Generic;
using System.Linq;
using LamestWebserver;
using LamestWebserver.UI;
using LamestWebserver.Core;

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

    yield return new HLink("Products", "/subsystem") { Class = "nav" };
    yield return new HLink(productName, $"/subsystem?p={productName.EncodeUrl()}") { Class = "nav" };
    yield return new HLink(majorVersion.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}") { Class = "nav" };
    yield return new HLink(minorVersion.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}") { Class = "nav" };
    yield return new HLink(info.GetSubSystemName(subSystem), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={subSystem}") { Class = "nav" };

    var s = analysis.subSystems.FindItem((uint64_t)subSystem).states.FindItem(new StateId(state, subStateIndex));

    yield return new HHeadline(info.GetStateName((uint64_t)subSystem, (uint64_t)state, (uint64_t)subStateIndex)) { Class = "stateName" };

    yield return GraphGen.DisplayInfo(s);
    yield return GraphGen.ToPieChart(analysis, (uint64_t)subSystem, s.nextState, "Next State", info);
    yield return GraphGen.ToPieChart(analysis, (uint64_t)subSystem, s.previousState, "Previous State", info);
    yield return GraphGen.ToPieChart(analysis, (uint64_t)subSystem, s.operations, "Operations", info);
    yield return GraphGen.ToPieChart(analysis, (uint64_t)subSystem, s.previousOperation, "Previous Operation", info);
    yield return GraphGen.ToHistorgramChart(analysis, (uint64_t)subSystem, s.profileData, "Performance", info);

    yield return GraphGen.ToErrorChart(s.errors, "Errors");
    yield return GraphGen.ToErrorChart(s.warnings, "Warnings");
    yield return GraphGen.ToPieChart(s.logs.values, "Logs", info, s.logs.count);

    yield return new HHeadline($"Observed Values UInt64", 2);

    if (s.observedU64 != null)
      foreach (var x in s.observedU64)
        yield return new HLink(info.GetValueNameU64(x.index.value), $"/value?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=u64&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Values Int64", 2);

    if (s.observedI64 != null)
      foreach (var x in s.observedI64)
        yield return new HLink(info.GetValueNameI64(x.index.value), $"/value?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=i64&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Values String", 2);

    if (s.observedString != null)
      foreach (var x in s.observedString)
        yield return new HLink(info.GetValueNameString(x.index.value), $"/value?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=string&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Value Range UInt64", 2);

    if (s.observedRangeU64 != null)
      foreach (var x in s.observedRangeU64)
        yield return new HLink(info.GetValueRangeNameU64(x.index.value), $"/range?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=u64&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Value Range Int64", 2);

    if (s.observedRangeI64 != null)
      foreach (var x in s.observedRangeI64)
        yield return new HLink(info.GetValueRangeNameI64(x.index.value), $"/range?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=i64&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Value Range Float64", 2);

    if (s.observedRangeF64 != null)
      foreach (var x in s.observedRangeF64)
        yield return new HLink(info.GetValueRangeNameF64(x.index.value), $"/range?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=f64&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Value Range Vector2<Float32>", 2);

    if (s.observedRangeF32_2 != null)
      foreach (var x in s.observedRangeF32_2)
        yield return new HLink(info.GetValueRangeNameF32_2(x.index.value), $"/range?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=f32_2&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return GraphGen.ToBarGraph(analysis, (uint64_t)subSystem, s.stateReach, "State Reach", info);
    yield return GraphGen.ToBarGraph(analysis, (uint64_t)subSystem, s.operationReach, "Operation Reach", info);

    // Display States with relevance information.
    if (s.nextState != null && s.previousState != null)
    {
      yield return new HHeadline("State Overview", 2) { ID = "overview" };

      List<List<HElement>> incoming = new List<List<HElement>>();
      List<List<HElement>> outgoing = new List<List<HElement>>();

      var list = from k in s.previousState.OrderByDescending(e => e.value.avgDelay) select new { state = k, usage = GetUsageInfoNext(analysis, (uint64_t)subSystem, k.index, new StateId(state, subStateIndex)) };
      ulong maxShare = list.Max(e => e.usage.Item2);
      ulong sumFromSelf = (ulong)list.Sum(e => (long)(ulong)e.state.value.count);
      ulong maxFromSelf = list.Max(e => (ulong)e.state.value.count);

      foreach (var x in list)
      {
        var stateLink = new HLink(info.GetStateName((uint64_t)subSystem, x.state.index.state, x.state.index.subState).WithMaxLength(35), $"/state?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={subSystem}&id={x.state.index.state}&sid={x.state.index.subState}#overview") { Class = "InState", Title = info.GetStateName((uint64_t)subSystem, x.state.index.state, x.state.index.subState), Style = $"--w:{x.usage.Item2 / (double)maxShare * 100.0};", Elements = { new HText($"{x.state.value.avgDelay:0.##} s | {x.usage.Item2}") } };
        var stateTransition = new HText($"{x.usage.Item1 / (double)x.usage.Item2 * 100.0:0.##} %") { Class = "InStateTrans", Style = $"--w:{x.usage.Item2 / (double)maxShare * 100.0};--p:{x.usage.Item1 / (double)x.usage.Item2}" };
        var fromThis = new HText($"{x.state.value.count / (double)sumFromSelf * 100.0:0.##} %") { Class = "OutStateTrans", Style = $"--w:{x.state.value.count / (double)maxFromSelf * 100.0};--p:{maxFromSelf / (double)sumFromSelf}" };

        incoming.Add(new List<HElement>() { stateLink, stateTransition, fromThis });
      }

      list = from k in s.nextState.OrderByDescending(e => e.value.avgDelay) select new { state = k, usage = GetUsageInfoPrevious(analysis, (uint64_t)subSystem, k.index, new StateId(state, subStateIndex)) };
      maxShare = list.Max(e => e.usage.Item2);
      sumFromSelf = (ulong)list.Sum(e => (long)(ulong)e.state.value.count);
      maxFromSelf = list.Max(e => (ulong)e.state.value.count);

      foreach (var x in list)
      {
        var stateLink = new HLink(info.GetStateName((uint64_t)subSystem, x.state.index.state, x.state.index.subState).WithMaxLength(35), $"/state?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={subSystem}&id={x.state.index.state}&sid={x.state.index.subState}#overview") { Class = "OutState", Title = info.GetStateName((uint64_t)subSystem, x.state.index.state, x.state.index.subState), Style = $"--w:{x.usage.Item2 / (double)maxShare * 100.0};", Elements = { new HText($"{x.state.value.avgDelay:0.##} s | {x.usage.Item2}") } };
        var stateTransition = new HText($"{x.usage.Item1 / (double)x.usage.Item2 * 100.0:0.##} %") { Class = "OutStateTrans", Style = $"--w:{x.usage.Item2 / (double)maxShare * 100.0};--p:{x.usage.Item1 / (double)x.usage.Item2}" };
        var fromThis = new HText($"{x.state.value.count / (double)sumFromSelf * 100.0:0.##} %") { Class = "InStateTrans", Style = $"--w:{x.state.value.count / (double)maxFromSelf * 100.0};--p:{maxFromSelf / (double)sumFromSelf}" };
        
        outgoing.Add(new List<HElement>() { fromThis, stateTransition, stateLink });
      }

      yield return new HTable(new List<List<HElement>>() { new List<HElement>() { new HTable(incoming) { Class = "InStateTab" }, new HTable(outgoing) { Class = "OutStateTab" } } }) { Class = "StateTransitions" };
    }
  }

  private static Tuple<ulong, ulong> GetUsageInfoNext(Analysis analysis, uint64_t subSystem, StateId index, StateId self)
  {
    ulong total = 0;
    ulong share = 0;

    try
    {
      var ps = analysis.subSystems.FindItem(subSystem).states.FindItem(new StateId(index.state, index.subState));

      total = (ulong)ps.nextState.Sum(x => (long)(ulong)x.value.count);
      share = (from y in ps.nextState where y.index.state == self.state && y.index.subState == self.subState select (ulong)y.value.count).FirstOrDefault();
    }
    catch { }

    return new Tuple<ulong, ulong>(share, total);
  }

  private static Tuple<ulong, ulong> GetUsageInfoPrevious(Analysis analysis, uint64_t subSystem, StateId index, StateId self)
  {
    ulong total = 0;
    ulong share = 0;

    try
    {
      var ps = analysis.subSystems.FindItem(subSystem).states.FindItem(new StateId(index.state, index.subState));

      total = (ulong)ps.previousState.Sum(x => (long)(ulong)x.value.count);
      share = (from y in ps.previousState where y.index.state == self.state && y.index.subState == self.subState select (ulong)y.value.count).FirstOrDefault();
    }
    catch { }

    return new Tuple<ulong, ulong>(share, total);
  }
}
