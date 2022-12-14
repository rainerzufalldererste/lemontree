using System.Collections.Generic;
using LamestWebserver;
using LamestWebserver.UI;
using LamestWebserver.Core;

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
      ulong subSystem, stateIndex, subStateIndex;

      if (!ulong.TryParse(sessionData.HttpHeadVariables["ss"], out subSystem) || !ulong.TryParse(sessionData.HttpHeadVariables["sid"], out stateIndex) || !ulong.TryParse(sessionData.HttpHeadVariables["ssid"], out subStateIndex))
      {
        switch (valueType)
        {
          case "u64": return ltsrv.GetPage("Value Info", ShowValueU64(sessionData, productName, majorVersion, minorVersion, valueIndex));
          case "i64": return ltsrv.GetPage("Value Info", ShowValueI64(sessionData, productName, majorVersion, minorVersion, valueIndex));
          case "string": return ltsrv.GetPage("Value Info", ShowValueString(sessionData, productName, majorVersion, minorVersion, valueIndex));
          default: return SubSystemInfo.GetMenu(sessionData);
        }
      }
      else
      {
        switch (valueType)
        {
          case "u64": return ltsrv.GetPage("Value Info", ShowValueU64(sessionData, productName, majorVersion, minorVersion, valueIndex, subSystem, stateIndex, subStateIndex));
          case "i64": return ltsrv.GetPage("Value Info", ShowValueI64(sessionData, productName, majorVersion, minorVersion, valueIndex, subSystem, stateIndex, subStateIndex));
          case "string": return ltsrv.GetPage("Value Info", ShowValueString(sessionData, productName, majorVersion, minorVersion, valueIndex, subSystem, stateIndex, subStateIndex));
          default: return SubSystemInfo.GetMenu(sessionData);
        }
      }
    }
  }

  private IEnumerable<HElement> ShowValues<T>(Info info, ExactValueData<T> data, ulong valueIndex)
  {
    if (data is ExactValueDataWithAverage<T>)
      yield return GraphGen.DisplayInfo(data as ExactValueDataWithAverage<T>);

    yield return GraphGen.DisplayInfo(data.data);
    yield return GraphGen.ToPieChart(data.values, "Values", info, data.count);
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

    yield return new HHeadline(info.GetValueNameU64((uint64_t)valueIndex)) { Class = "stateName" };

    var s = analysis.observedU64.FindItem((uint64_t)valueIndex);

    foreach (var x in ShowValues(info, s, valueIndex))
      yield return x;
  }

  private IEnumerable<HElement> ShowValueI64(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex)
  {
    foreach (var x in Breadcrumbs(productName, majorVersion, minorVersion))
      yield return x;

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline(info.GetValueNameI64((uint64_t)valueIndex)) { Class = "stateName" };

    var s = analysis.observedI64.FindItem((uint64_t)valueIndex);

    foreach (var x in ShowValues(info, s, valueIndex))
      yield return x;
  }

  private IEnumerable<HElement> ShowValueString(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex)
  {
    foreach (var x in Breadcrumbs(productName, majorVersion, minorVersion))
      yield return x;

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline(info.GetValueNameString((uint64_t)valueIndex)) { Class = "stateName" };

    var s = analysis.observedString.FindItem((uint64_t)valueIndex);

    foreach (var x in ShowValues(info, s, valueIndex))
      yield return x;
  }

  private IEnumerable<HElement> ShowValueU64(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex, ulong subSystem, ulong stateIndex, ulong subStateIndex)
  {
    foreach (var x in Breadcrumbs(productName, majorVersion, minorVersion, subSystem, stateIndex, subStateIndex))
      yield return x;

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline(info.GetValueNameU64((uint64_t)valueIndex)) { Class = "stateName" };

    var s = analysis.subSystems.FindItem((uint64_t)subStateIndex).states.FindItem(new StateId(stateIndex, subStateIndex)).observedU64.FindItem((uint64_t)valueIndex);

    foreach (var x in ShowValues(info, s, valueIndex))
      yield return x;
  }

  private IEnumerable<HElement> ShowValueI64(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex, ulong subSystem, ulong stateIndex, ulong subStateIndex)
  {
    foreach (var x in Breadcrumbs(productName, majorVersion, minorVersion, subSystem, stateIndex, subStateIndex))
      yield return x;

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline(info.GetValueNameI64((uint64_t)valueIndex)) { Class = "stateName" };

    var s = analysis.subSystems.FindItem((uint64_t)subStateIndex).states.FindItem(new StateId(stateIndex, subStateIndex)).observedI64.FindItem((uint64_t)valueIndex);

    foreach (var x in ShowValues(info, s, valueIndex))
      yield return x;
  }

  private IEnumerable<HElement> ShowValueString(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex, ulong subSystem, ulong stateIndex, ulong subStateIndex)
  {
    foreach (var x in Breadcrumbs(productName, majorVersion, minorVersion, subSystem, stateIndex, subStateIndex))
      yield return x;

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    yield return new HHeadline(info.GetValueNameString((uint64_t)valueIndex)) { Class = "stateName" };

    var s = analysis.subSystems.FindItem((uint64_t)subStateIndex).states.FindItem(new StateId(stateIndex, subStateIndex)).observedString.FindItem((uint64_t)valueIndex);

    foreach (var x in ShowValues(info, s, valueIndex))
      yield return x;
  }
}
