using System;
using System.Collections.Generic;

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

  public static double ToDouble(this object d)
  {
    if (d is double)
      return (double)d;
    else if (d is uint64_t)
      return (double)((uint64_t)d).value;
    else if (d is int64_t)
      return (double)((int64_t)d).value;
    else
      return Convert.ToDouble(d);
  }
  public static string WithMaxLength(this string value, int maxLength)
  {
    if (string.IsNullOrEmpty(value))
      return "";

    if (value.Length > maxLength - 3)
      return value.Substring(0, System.Math.Min(value.Length, Math.Max(0, maxLength - 3))) + "...";
    else
      return value;
  }
}
