using System.Collections.Generic;
using LamestWebserver;
using LamestWebserver.UI;
using LamestWebserver.Core;

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

    yield return new HLink("Products", "/subsystem") { Class = "nav" };
    yield return new HLink(productName, $"/subsystem?p={productName.EncodeUrl()}") { Class = "nav" };
    yield return new HLink(majorVersion.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}") { Class = "nav" };
    yield return new HLink(minorVersion.ToString(), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}") { Class = "nav" };
    yield return new HLink(info.GetSubSystemName(subSystem), $"/subsystem?p={productName.EncodeUrl()}&V={majorVersion}&v={minorVersion}&ss={subSystem}") { Class = "nav" };

    var s = analysis.subSystems.FindItem((uint64_t)subSystem).operations.FindItem((uint64_t)operationIndex);

    yield return new HHeadline(info.GetOperationName((uint64_t)subSystem, (uint64_t)operationIndex)) { Class = "stateName" };

    yield return GraphGen.DisplayInfo(s);
    yield return GraphGen.ToPieChart(s.operationIndexCount, "Operation Index");
    yield return GraphGen.ToPieChart(analysis, (uint64_t)subSystem, s.nextOperation, "Next Operation", info);
    yield return GraphGen.ToPieChart(analysis, (uint64_t)subSystem, s.parentState, "Parent State", info);
    yield return GraphGen.ToPieChart(analysis, (uint64_t)subSystem, s.nextState, "Next State", info);
    yield return GraphGen.ToPieChart(analysis, (uint64_t)subSystem, s.lastOperation, "Previous Operation", info);
    
    yield return GraphGen.ToErrorChart(analysis, (uint64_t)subSystem, s.errors, "Errors", true);
    yield return GraphGen.ToErrorChart(analysis, (uint64_t)subSystem, s.warnings, "Warnings", false);
    yield return GraphGen.ToPieChart(s.logs.values, "Logs", info, s.logs.count);
  }
}
