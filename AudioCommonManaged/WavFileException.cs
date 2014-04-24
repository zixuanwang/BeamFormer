using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AudioCommonManaged
{
    public class WavFileException : Exception
    {
        public WavFileException() { }
        public WavFileException(string message) : base(message) { }
    }
}
