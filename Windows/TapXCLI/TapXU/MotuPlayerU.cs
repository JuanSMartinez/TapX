using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using TapXCLI;

namespace TapXU
{
    public class MotuPlayerU
    {
        //Singleton
        private static MotuPlayerU instance = null;

        //The CLI player
        MotuPlayerCLI player;

        //Lock object 
        private static readonly object lockObject = new object();

        //Instance of the class
        public static MotuPlayerU Instance
        {
            get
            {
                lock(lockObject)
                {
                    if (instance == null)
                    {
                        instance = new MotuPlayerU();
                    }
                    return instance;
                }
            }
        }

        //Initialized flag
        public bool Initialized { get; private set; }

        //Constructor
        private MotuPlayerU()
        {
            Thread startThread = new Thread(new ThreadStart(Initialize));
            startThread.Start();
        }

        //Initialize routine
        private void Initialize()
        {
            Initialized = false;
            player = new MotuPlayerCLI();
            Initialized = true;
        }

        //Play a haptic symbol
        public void PlayHapticSymbol(string symbol)
        {
            if(Initialized)
            {
                player.PlayHapticSymbol(symbol);
            }
        }
    }
}
