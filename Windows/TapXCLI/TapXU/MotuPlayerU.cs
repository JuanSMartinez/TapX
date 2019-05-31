using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Runtime.InteropServices;

namespace TapXU
{
    public class MotuPlayerU
    {
        //DLL imports from native code
        [DllImport("TapXCoreExport")]
        static extern void createMotuPlayer();
        [DllImport("TapXCoreExport")]
        static extern void finalize();

        //Singleton
        private static MotuPlayerU instance = null;

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

        //Finalizer
        ~MotuPlayerU()
        {
            finalize();
        }

        //Initialize routine
        private void Initialize()
        {
            Initialized = false;
            createMotuPlayer();
            Initialized = true;
        }

        //Play a haptic symbol
        public void PlayHapticSymbol(string symbol)
        {
            if(Initialized)
            {
                
            }
        }
    }
}
