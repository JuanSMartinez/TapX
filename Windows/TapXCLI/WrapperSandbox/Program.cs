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
        public void SequenceCallback(int err)
        {
            Console.WriteLine("Played sequence with code " + err);
        }

        public void SymbolCallback(int err)
        {
            Console.WriteLine("Played symbol with code " + err);
        }

        public void StartFlagCallback(int err)
        {
            Console.WriteLine("Played start flag with code " + err);
        }

        static void Main(string[] args)
        {
            string input;
            string options = "Type one of the following:\n" +
                             "To play a single symbol: 1,<SYMBOL>\n" +
                             "To play a sequence of symbols with an ICI (ms): 2,<SEQUENCE SEPARATED BY COMAS>,<ICI>\n"+
                             "To play an English sentence with an ICI (ms) and IWI (ms): 3,<SENTENCE>,<ICI>,<IWI>\n" +
                             "To play a sequence of symbols with an ICI (ms) and a KNOCK start flag: 4,<SEQUENCE SEPARATED BY COMAS>,<ICI>\n" +
                             "To play an English sentence with an ICI (ms), IWI (ms) and KNOCK start flag: 5,<SENTENCE>,<ICI>,<IWI>\n" +
                             "To exit: <XX>";

            Program p = new Program();
            Console.WriteLine("Creating player...");
            MotuPlayerCLI player = new MotuPlayerCLI();
            Console.WriteLine("Created player");

            if(player.SessionStarted())
            {
                player.RegisterExternalSymbolCallback(p.SymbolCallback);
                player.RegisterExternalSequenceCallback(p.SequenceCallback);
                Console.WriteLine(options);
                input = Console.ReadLine().Trim();
                while (!input.Equals("XX"))
                {
                    char selection = input[0];
                    double num = char.GetNumericValue(selection);
                    string[] data;
                    string[] symbols;
                    int ici, iwi;
                    switch (num)
                    {
                        case 1:
                            string symbol = input.Split(new char[] { ',' })[1];
                            Console.WriteLine("Playing " + symbol);
                            player.PlayHapticSymbol(symbol);
                            break;
                        case 2:
                            data = input.Split(new char[] { ',' });
                            ici = int.Parse(data[data.Length - 1]);
                            symbols = new string[data.Length - 2];
                            for (int i = 1; i < data.Length - 1; i++)
                            {
                                symbols[i - 1] = data[i];
                            }
                            Console.WriteLine("Playing the sequence with " + ici + " ms");
                            player.PlaySequenceOfSymbols(symbols, ici);
                            break;
                        case 3:
                            data = input.Split(new char[] { ',' });
                            ici = int.Parse(data[2]);
                            iwi = int.Parse(data[3]);
                            Console.WriteLine("Playing the sentence '" + data[1] + "' with " + ici + " ms of ICI and " + iwi + " ms of IWI");
                            player.PlayEnglishSentence(data[1], ici, iwi);
                            break;
                        case 4:
                            data = input.Split(new char[] { ',' });
                            ici = int.Parse(data[data.Length - 1]);
                            symbols = new string[data.Length - 2];
                            for (int i = 1; i < data.Length - 1; i++)
                            {
                                symbols[i - 1] = data[i];
                            }
                            Console.WriteLine("Playing the sequence with " + ici + " ms");
                            player.PlaySequenceOfSymbols(symbols, ici, p.StartFlagCallback, "KNOCK");
                            break;
                        case 5:
                            data = input.Split(new char[] { ',' });
                            ici = int.Parse(data[2]);
                            iwi = int.Parse(data[3]);
                            Console.WriteLine("Playing the sentence '" + data[1] + "' with " + ici + " ms of ICI and " + iwi + " ms of IWI");
                            player.PlayEnglishSentence(data[1], ici, iwi, p.StartFlagCallback, "KNOCK");
                            break;
                        default:
                            Console.WriteLine("Default");
                            break;
                    }


                    Console.WriteLine(options);
                    input = Console.ReadLine().Trim();
                }
            }

        }
    }
}
