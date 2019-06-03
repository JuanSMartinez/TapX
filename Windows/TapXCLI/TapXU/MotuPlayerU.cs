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
        //Callback delegates
        public delegate void SymbolCallback(int error);
        public delegate void SequenceCallback(int error);

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
        [DllImport("TapXCoreExport")]
        static extern void registerExternalSymbolCallback(SymbolCallback callback);
        [DllImport("TapXCoreExport")]
        static extern void registerExternalSequenceCallback(SequenceCallback callback);

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

        //Set a symbol played callback
        public void SetSymbolCallback(SymbolCallback callback)
        {
            if(Initialized)
                registerExternalSymbolCallback(callback);
        }

        //Set a sequence played callback
        public void SetSequenceCallback(SequenceCallback callback)
        {
            if (Initialized)
                registerExternalSequenceCallback(callback);
        }

    }
}
