#include "../inc/MotuPlayer.hpp"
using namespace TapX;

void testSymbolCallback(TapsError err)
{
    printf("External callback used, played symbol with code %d\n", err);
}

void testSequenceCallback(TapsError err)
{
    printf("External callback for sequence used, played sequence with code %d\n", err);
}

int playIndividualSymbols()
{
    MotuPlayer* player = MotuPlayer::getInstance();
    player->registerSymbolPlayedCallback(testSymbolCallback);
    if(player->successfulStart())
    {
        char buff[5];
        printf("Type a symbol label or 'xx' to exit\n");
        scanf("%5s", buff);
        while (std::string(buff).compare("xx") != 0)
        {
            player->playHapticSymbol(std::string(buff));
            
            printf("Type a phoneme label to play or 'xx' to exit\n");
            scanf("%5s", buff);
        }
        delete player;
        return 0;
    }
    return -1;
}

int playSequenceOfSymbols()
{
    MotuPlayer* player = MotuPlayer::getInstance();
    player->registerSequencePlayedCallback(testSequenceCallback);
    if(player->successfulStart())
    {
        char buff[64];
        printf("Type a sequence of symbols separated by comas and ICI at the end or 'xx' to exit\n");
        scanf("%64s", buff);
        std::vector<std::string> vector;
        while (std::string(buff).compare("xx") != 0)
        {
            vector.clear();
            std::string typed(buff);
            
            size_t pos = 0;
            std::string token;
            std::string delimiter = ",";
            while((pos = typed.find(delimiter)) != std::string::npos)
            {
                token = typed.substr(0,pos);
                vector.push_back(token);
                typed.erase(0, pos + delimiter.length());
            }
            int ici = (int)strtol(typed.c_str(), NULL, 10);

            printf("Playing sequence of symbols with %d ms ICI\n", ici);
            player->playSymbolSequence(vector, ici);
            
            printf("Type a sequence of symbols separated by comas and ICI at the end or 'xx' to exit\n");
            scanf("%64s", buff);
        }
        delete player;
        return 0;

    }
    return -1;
}

int getRawFlitePhonemes()
{
    MotuPlayer* player = MotuPlayer::getInstance();
    if(player->successfulStart())
    {
        char buff[64];
        std::string result;
        printf("Type a sentence or 'xx' to exit\n");
        fgets(buff, 64, stdin);
        if ((strlen(buff) > 0) && (buff[strlen (buff) - 1] == '\n'))
            buff[strlen (buff) - 1] = '\0';
        while (std::string(buff).compare("xx") != 0)
        {
            std::string typed(buff);
            
            result = player->getRawFlitePhonemes(typed);
            printf("Result: %s\n", result.c_str());
            
            printf("Type a sentence or 'xx' to exit\n");
            fgets(buff, 64, stdin);
            if ((strlen(buff) > 0) && (buff[strlen (buff) - 1] == '\n'))
                buff[strlen (buff) - 1] = '\0';
        }
        delete player;
        return 0;

    }
    return -1;
}

int getPhonemes()
{
    MotuPlayer* player = MotuPlayer::getInstance();
    std::vector<std::string> result;
    if(player->successfulStart())
    {
        char buff[64];
        std::vector<std::string>::iterator it;
        printf("Type a sentence or 'xx' to exit\n");
        fgets(buff, 64, stdin);
        if ((strlen(buff) > 0) && (buff[strlen (buff) - 1] == '\n'))
            buff[strlen (buff) - 1] = '\0';
        while (std::string(buff).compare("xx") != 0)
        {
            std::string typed(buff);
            result.clear();
            player->getPhonemesOfSentence(&result, typed);
            
            it = result.begin();
            while(it != result.end())
            {
                printf("Phoneme: %s\n", (*it).c_str());
                it++;
            }
            
            
            printf("Type a sentence or 'xx' to exit\n");
            fgets(buff, 64, stdin);
            if ((strlen(buff) > 0) && (buff[strlen (buff) - 1] == '\n'))
                buff[strlen (buff) - 1] = '\0';
        }
        delete player;
        return 0;

    }
    return -1;
}

int playSentence()
{
    MotuPlayer* player = MotuPlayer::getInstance();
    player->registerSequencePlayedCallback(testSequenceCallback);
    if(player->successfulStart())
    {
        char buff[64];
        std::vector<std::string> result;
        std::vector<std::string>::iterator it;
        printf("Type a sentence to play or 'xx' to exit\n");
        fgets(buff, 64, stdin);
        if ((strlen(buff) > 0) && (buff[strlen (buff) - 1] == '\n'))
            buff[strlen (buff) - 1] = '\0';
        while (std::string(buff).compare("xx") != 0)
        {
            std::string typed(buff);
            
            player->playEnglishSentence(typed, 150, 1500);
            
            printf("Type a sentence to play or 'xx' to exit\n");
            fgets(buff, 64, stdin);
            if ((strlen(buff) > 0) && (buff[strlen (buff) - 1] == '\n'))
                buff[strlen (buff) - 1] = '\0';
        }
        delete player;
        return 0;

    }
    return -1;
}

int main( int argc, char *argv[] )
{
    if(argc == 2)
    {
        printf("Creating motu player\n");
        MotuPlayer* player = MotuPlayer::getInstance();
        player->startSession();
        
        if(strtol(argv[1], NULL,10) == 1)
            return playIndividualSymbols();
        else if(strtol(argv[1], NULL, 10) == 2)
            return playSequenceOfSymbols();
        else if(strtol(argv[1], NULL, 10) == 3)
            return getRawFlitePhonemes();
        else if(strtol(argv[1], NULL, 10) == 4)
            return getPhonemes();
        else if(strtol(argv[1], NULL, 10) == 5)
            return playSentence();
        else
        {
            printf("Not yet implemented\n");
            delete player;
            return 0;
        }
        
    }
    else
    {
        printf("Incorrect number of arguments\n");
        return -1;
    }
    
}