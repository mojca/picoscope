using System;
using System.Runtime.InteropServices;

namespace ps6000example
{
  public class PinnedArray<T>
  {
    GCHandle _pinnedHandle;
    private bool _disposed;

    public PinnedArray(int arraySize) : this(new T[arraySize]) { }

    public PinnedArray(T[] array)
    {
      _pinnedHandle = GCHandle.Alloc(array, GCHandleType.Pinned);
    }

    ~PinnedArray()
    {
      Dispose();
    }

    public T[] Target
    {
      get { return (T[])_pinnedHandle.Target; }
    }

    public static implicit operator T[](PinnedArray<T> a)
    {
      if (a == null)
        return null;
      else
        return (T[])a._pinnedHandle.Target;
    }

    public void Dispose()
    {
      if (!_disposed)
      {
        _pinnedHandle.Free();
        _disposed = true;

        GC.SuppressFinalize(this);
      }
    }
  }
}