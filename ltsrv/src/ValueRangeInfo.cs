using System.Collections.Generic;
using LamestWebserver;
using LamestWebserver.UI;
using LamestWebserver.Core;

public class ValueRangeInfo : ElementResponse
{
  public ValueRangeInfo() : base("range") { }

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
      ulong subSystem, stateIndex, subStateIndex;

      if (!ulong.TryParse(sessionData.HttpHeadVariables["ss"], out subSystem) || !ulong.TryParse(sessionData.HttpHeadVariables["sid"], out stateIndex) || !ulong.TryParse(sessionData.HttpHeadVariables["ssid"], out subStateIndex))
      {
        switch (valueType)
        {
          case "u64": return ltsrv.GetPage("Value Info", ShowValueU64(sessionData, productName, majorVersion, minorVersion, valueIndex));
          case "i64": return ltsrv.GetPage("Value Info", ShowValueI64(sessionData, productName, majorVersion, minorVersion, valueIndex));
          case "f64": return ltsrv.GetPage("Value Info", ShowValueF64(sessionData, productName, majorVersion, minorVersion, valueIndex));
          case "f32_2": return ltsrv.GetPage("Value Info", ShowValueF32_2(sessionData, productName, majorVersion, minorVersion, valueIndex));
          default: return SubSystemInfo.GetMenu(sessionData);
        }
      }
      else
      {
        switch (valueType)
        {
          case "u64": return ltsrv.GetPage("Value Info", ShowValueU64(sessionData, productName, majorVersion, minorVersion, valueIndex, subSystem, stateIndex, subStateIndex));
          case "i64": return ltsrv.GetPage("Value Info", ShowValueI64(sessionData, productName, majorVersion, minorVersion, valueIndex, subSystem, stateIndex, subStateIndex));
          case "f64": return ltsrv.GetPage("Value Info", ShowValueF64(sessionData, productName, majorVersion, minorVersion, valueIndex, subSystem, stateIndex, subStateIndex));
          case "f32_2": return ltsrv.GetPage("Value Info", ShowValueF32_2(sessionData, productName, majorVersion, minorVersion, valueIndex, subSystem, stateIndex, subStateIndex));
          default: return SubSystemInfo.GetMenu(sessionData);
        }
      }
    }
  }

  private IEnumerable<HElement> Breadcrumbs(string productName, ulong majorVersion, ulong minorVersion)
  {
    yield return new HLink("Products", "/subsystem") { Class = "nav" };
    yield return new HLink(productName, $"/subsystem?p={productName.EncodeUrl()}") { Class = "nav" };
    yield return new HLink(majorVersion.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}") { Class = "nav" };
    yield return new HLink(minorVersion.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}") { Class = "nav" };
  }

  private IEnumerable<HElement> Breadcrumbs(string productName, ulong majorVersion, ulong minorVersion, ulong subSystem, ulong stateIndex, ulong subStateIndex)
  {
    var info = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion].info;

    yield return new HLink("Products", "/subsystem") { Class = "nav" };
    yield return new HLink(productName, $"/subsystem?p={productName.EncodeUrl()}") { Class = "nav" };
    yield return new HLink(majorVersion.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}") { Class = "nav" };
    yield return new HLink(minorVersion.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}") { Class = "nav" };
    yield return new HLink(info.GetSubSystemName(subSystem), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={subSystem}") { Class = "nav" };
    yield return new HLink(info.GetStateName((uint64_t)subSystem, (uint64_t)stateIndex, (uint64_t)subStateIndex), $"/state?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={subSystem}&id={stateIndex}&sid={subStateIndex}") { Class = "nav" };
  }

  private IEnumerable<HElement> ShowValueU64(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex)
  {
    foreach (var x in Breadcrumbs(productName, majorVersion, minorVersion))
      yield return x;

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline(info.GetValueRangeNameU64((uint64_t)valueIndex)) { Class = "stateName" };

    var s = analysis.observedRangeU64.FindItem((uint64_t)valueIndex);

    yield return GraphGen.ToHistorgramChart(s, "Info");
  }

  private IEnumerable<HElement> ShowValueI64(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex)
  {
    foreach (var x in Breadcrumbs(productName, majorVersion, minorVersion))
      yield return x;

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline(info.GetValueRangeNameI64((uint64_t)valueIndex)) { Class = "stateName" };

    var s = analysis.observedRangeI64.FindItem((uint64_t)valueIndex);

    yield return GraphGen.ToHistorgramChart(s, "Info");
  }

  private IEnumerable<HElement> ShowValueF64(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex)
  {
    foreach (var x in Breadcrumbs(productName, majorVersion, minorVersion))
      yield return x;

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    var s = analysis.observedRangeF64.FindItem((uint64_t)valueIndex);

    yield return new HHeadline(info.GetValueRangeNameF64((uint64_t)valueIndex)) { Class = "stateName" };

    yield return GraphGen.ToHistorgramChart(s, "Info");
  }

  private IEnumerable<HElement> ShowValueF32_2(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex)
  {
    foreach (var x in Breadcrumbs(productName, majorVersion, minorVersion))
      yield return x;

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline(info.GetValueRangeNameF32_2((uint64_t)valueIndex)) { Class = "stateName" };

    var s = analysis.observedRangeF32_2.FindItem((uint64_t)valueIndex);

    yield return GraphGen.To2DHistorgramChart(s, "Info");
  }

  private IEnumerable<HElement> ShowValueU64(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex, ulong subSystem, ulong stateIndex, ulong subStateIndex)
  {
    foreach (var x in Breadcrumbs(productName, majorVersion, minorVersion, subSystem, stateIndex, subStateIndex))
      yield return x;

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline(info.GetValueRangeNameU64((uint64_t)valueIndex)) { Class = "stateName" };

    var s = analysis.subSystems.FindItem((uint64_t)subStateIndex).states.FindItem(new StateId(stateIndex, subStateIndex)).observedRangeU64.FindItem((uint64_t)valueIndex);

    yield return GraphGen.ToHistorgramChart(s, "Info");
  }

  private IEnumerable<HElement> ShowValueI64(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex, ulong subSystem, ulong stateIndex, ulong subStateIndex)
  {
    foreach (var x in Breadcrumbs(productName, majorVersion, minorVersion, subSystem, stateIndex, subStateIndex))
      yield return x;

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline(info.GetValueRangeNameI64((uint64_t)valueIndex)) { Class = "stateName" };

    var s = analysis.subSystems.FindItem((uint64_t)subStateIndex).states.FindItem(new StateId(stateIndex, subStateIndex)).observedRangeI64.FindItem((uint64_t)valueIndex);

    yield return GraphGen.ToHistorgramChart(s, "Info");
  }

  private IEnumerable<HElement> ShowValueF64(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex, ulong subSystem, ulong stateIndex, ulong subStateIndex)
  {
    foreach (var x in Breadcrumbs(productName, majorVersion, minorVersion, subSystem, stateIndex, subStateIndex))
      yield return x;

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    var s = analysis.observedRangeF64.FindItem((uint64_t)valueIndex);

    yield return new HHeadline(info.GetValueRangeNameF64((uint64_t)valueIndex)) { Class = "stateName" };

    yield return GraphGen.ToHistorgramChart(s, "Info");
  }

  private IEnumerable<HElement> ShowValueF32_2(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex, ulong subSystem, ulong stateIndex, ulong subStateIndex)
  {
    foreach (var x in Breadcrumbs(productName, majorVersion, minorVersion, subSystem, stateIndex, subStateIndex))
      yield return x;

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline(info.GetValueRangeNameF32_2((uint64_t)valueIndex)) { Class = "stateName" };

    var s = analysis.subSystems.FindItem((uint64_t)subStateIndex).states.FindItem(new StateId(stateIndex, subStateIndex)).observedRangeF32_2.FindItem((uint64_t)valueIndex);

    yield return GraphGen.To2DHistorgramChart(s, "Info");
  }
}
