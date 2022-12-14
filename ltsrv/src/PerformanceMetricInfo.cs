using System.Collections.Generic;
using LamestWebserver;
using LamestWebserver.UI;
using LamestWebserver.Core;

public class PerformanceMetricInfo : ElementResponse
{
  public PerformanceMetricInfo() : base("perfmet") { }

  protected override HElement GetElement(SessionData sessionData)
  {
    string productName = sessionData.HttpHeadVariables["p"];
    ulong majorVersion, minorVersion, valueIndex = 0;

    if (productName == null || !ulong.TryParse(sessionData.HttpHeadVariables["V"], out majorVersion) || !ulong.TryParse(sessionData.HttpHeadVariables["v"], out minorVersion) || !ulong.TryParse(sessionData.HttpHeadVariables["id"], out valueIndex))
    {
      return SubSystemInfo.GetMenu(sessionData);
    }
    else
    {
      return ltsrv.GetPage("Value Info", ShowMetric(sessionData, productName, majorVersion, minorVersion, valueIndex));
    }
  }

  private IEnumerable<HElement> ShowMetric(SessionData sessionData, string productName, ulong majorVersion, ulong minorVersion, ulong valueIndex)
  {
    yield return new HLink("Products", "/subsystem") { Class = "nav" };
    yield return new HLink(productName, $"/subsystem?p={productName.EncodeUrl()}") { Class = "nav" };
    yield return new HLink(majorVersion.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}") { Class = "nav" };
    yield return new HLink(minorVersion.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}") { Class = "nav" };

    var container = ltsrv._Analysis[productName][(uint64_t)majorVersion][(uint64_t)minorVersion];
    var info = container.info;
    var analysis = container.analysis;

    var s = analysis.perfMetrics.FindItem((uint64_t)valueIndex);

    yield return new HHeadline(info.GetPerfMetricName((uint64_t)valueIndex)) { Class = "stateName" };

    yield return GraphGen.ToHistorgramChart(s, "Info");
  }
}
