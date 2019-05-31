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
        [DllImport("TapXCoreExport")]
        static extern void playHapticSymbol(string symbol);
        [DllImport("TapXCoreExport")]
        static extern void playSequence(string[] sequence, int sequenceLength, int ici);
        [DllImport("TapXCoreExport")]
        static extern void playEnglishSentence(string sentence, int ici, int iwi);

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
                playHapticSymbol(symbol);
            }
        }

        //Play a sequence of symbols
        public void PlaySequence(string[] sequence, int ici)
        {
            if(Initialized)
            {
                playSequence(sequence, sequence.Length, ici);
            }
        }

        //Play and English sentence
        public void PlayEnglishSentence(string sentence, int ici, int iwi)
        {
            if(Initialized)
            {
                playEnglishSentence(sentence, ici, iwi);
            }
        }

    }
}
