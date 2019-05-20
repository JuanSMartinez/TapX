#pragma once
#include <string>
#include <vector>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace TapX
{
    class HapticSymbol
    {
        private:
            std::string id;
            float* dataMatrix;
            size_t matrixRowIndex;
            size_t matrixRows;
        public:
            HapticSymbol(std::string symbolLabel);
            ~HapticSymbol();
            size_t getMatrixRowIndex () const;
            bool matrixConsumed() const;
            bool increaseIndex();
            void initializeData(std::string dataPath);
            void initializeData(int durationMs, int fs);
            void resetIndex();
            float getValueAt(int i, int j);
    };
}