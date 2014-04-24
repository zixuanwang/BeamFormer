using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AudioCommonManaged
{
    /// <summary>
    /// Provide bit conversions that do not depend on the host architecture.  It would be tedious
    /// to specify all of the conversions supported by BitConverter, so only the few we need are
    /// included here.  It would be nice if there were some way to do this generically...
    /// </summary>
    public static class EndianBitConverter
    {
        public static byte[] GetBytes(Int16 value, bool littleEndian)
        {
            byte[] bytes = BitConverter.GetBytes(value);
            if (BitConverter.IsLittleEndian != littleEndian)
            {
                Array.Reverse(bytes);
            }
            return bytes;
        }

        public static byte[] GetBytes(Int32 value, bool littleEndian)
        {
            byte[] bytes = BitConverter.GetBytes(value);
            if (BitConverter.IsLittleEndian != littleEndian)
            {
                Array.Reverse(bytes);
            }
            return bytes;
        }

        public static byte[] GetBytes(UInt32 value, bool littleEndian)
        {
            byte[] bytes = BitConverter.GetBytes(value);
            if (BitConverter.IsLittleEndian != littleEndian)
            {
                Array.Reverse(bytes);
            }
            return bytes;
        }

        public static Int16 ToInt16(byte[] bytes, int startIndex, bool littleEndian)
        {
            if (BitConverter.IsLittleEndian != littleEndian)
            {
                Array.Reverse(bytes);
            }
            return BitConverter.ToInt16(bytes, startIndex);
        }

        public static Int32 ToInt32(byte[] bytes, int startIndex, bool littleEndian)
        {
            if (BitConverter.IsLittleEndian != littleEndian)
            {
                Array.Reverse(bytes);
            }
            return BitConverter.ToInt32(bytes, startIndex);
        }

        public static UInt32 ToUInt32(byte[] bytes, int startIndex, bool littleEndian)
        {
            if (BitConverter.IsLittleEndian != littleEndian)
            {
                Array.Reverse(bytes);
            }
            return BitConverter.ToUInt32(bytes, startIndex);
        }
    }
}
