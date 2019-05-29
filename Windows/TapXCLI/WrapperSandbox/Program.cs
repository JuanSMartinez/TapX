using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using TapXCLI;

namespace WrapperSandbox
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Creating player...");
            MotuPlayerCLI player = new MotuPlayerCLI();
            Console.WriteLine("Created player");

            Console.WriteLine("Session started? " + player.SessionStarted());
            Console.Read();
        }
    }
}
