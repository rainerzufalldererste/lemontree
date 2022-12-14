using System.Collections.Generic;
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

    foreach (var x in s.observedU64)
      yield return new HLink(info.GetValueNameU64(x.index.value), $"/value?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=u64&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Values Int64", 2);

    foreach (var x in s.observedI64)
      yield return new HLink(info.GetValueNameI64(x.index.value), $"/value?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=i64&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Values String", 2);

    foreach (var x in s.observedString)
      yield return new HLink(info.GetValueNameString(x.index.value), $"/value?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=string&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Value Range UInt64", 2);

    foreach (var x in s.observedRangeU64)
      yield return new HLink(info.GetValueRangeNameU64(x.index.value), $"/range?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=u64&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Value Range Int64", 2);

    foreach (var x in s.observedRangeI64)
      yield return new HLink(info.GetValueRangeNameI64(x.index.value), $"/range?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=i64&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Value Range Float64", 2);

    foreach (var x in s.observedRangeF64)
      yield return new HLink(info.GetValueRangeNameF64(x.index.value), $"/range?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=f64&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return new HHeadline($"Observed Value Range Vector2<Float32>", 2);

    foreach (var x in s.observedRangeF32_2)
      yield return new HLink(info.GetValueRangeNameF32_2(x.index.value), $"/range?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&t=f32_2&id={x.index}&ss={subSystem}&sid={state}&ssid={subStateIndex}") { Class = "LargeButton" };

    yield return GraphGen.ToBarGraph(analysis, (uint64_t)subSystem, s.stateReach, "State Reach", info);
    yield return GraphGen.ToBarGraph(analysis, (uint64_t)subSystem, s.operationReach, "Operation Reach", info);
  }
}
