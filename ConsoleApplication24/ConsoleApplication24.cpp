#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstdio>

using namespace std;

const int BLOCK_SIZE = 1024;

// Копіює рядок з src у dst
void copyText(char* dst, const char* src) {
    while (*src) *dst++ = *src++;
    *dst = '\0';
}

// Повертає довжину рядка
int getLength(const char* text) {
    int length = 0;
    while (text[length]) length++;
    return length;
}

// Перевіряє, чи починається текст з шуканого слова
bool isWordHere(const char* text, const char* word) {
    int i = 0;
    while (word[i]) {
        if (text[i] != word[i]) return false;
        i++;
    }
    return true;
}

// Записує вивідний буфер у файл
void writeToFile(FILE* output, const char* buffer, int size) {
    fwrite(buffer, 1, size, output);
}

int main() {
    char fileName[100];
    char targetWord[100];
    char newWord[100];
    char mode;

    cout << "Enter file name: ";
    cin >> fileName;

    cout << "Enter word to search: ";
    cin >> targetWord;

    cout << "Choose action - (r)eplace, (d)elete or (s)earch only: ";
    cin >> mode;

    if (mode == 'r') {
        cout << "Enter replacement word: ";
        cin >> newWord;
    }
    else {
        newWord[0] = '\0';
    }

    FILE* input = fopen(fileName, "rb");
    if (!input) {
        cout << "Error: cannot open file.\n";
        return 1;
    }

    FILE* output = fopen("temp.tmp", "wb");
    if (!output) {
        cout << "Error: cannot create temp file.\n";
        fclose(input);
        return 1;
    }

    char inputBuffer[BLOCK_SIZE];
    char outputBuffer[BLOCK_SIZE * 2]; // на випадок, якщо слово буде замінено довшим 
    char leftover[100] = {};
    int leftoverLen = 0;

    int foundCount = 0;
    int bytesRead = 0;

    int wordLen = getLength(targetWord);
    int replaceLen = getLength(newWord);

    while ((bytesRead = fread(inputBuffer, 1, BLOCK_SIZE, input)) > 0) {
        char combined[BLOCK_SIZE + 100];
        int combinedLen = 0;

        // Додаємо залишок з попереднього блоку
        for (int i = 0; i < leftoverLen; i++)
            combined[combinedLen++] = leftover[i];
        for (int i = 0; i < bytesRead; i++)
            combined[combinedLen++] = inputBuffer[i];

        leftoverLen = 0;
        int outPos = 0;
        int i = 0;

        while (i <= combinedLen - wordLen) {
            if (isWordHere(&combined[i], targetWord)) {
                foundCount++;
                if (mode == 'r') {
                    for (int j = 0; j < replaceLen; j++)
                        outputBuffer[outPos++] = newWord[j];
                }
                // Якщо delete — не додаємо нічого
                i += wordLen;
            }
            else {
                outputBuffer[outPos++] = combined[i++];
            }
        }

        // Зберігаємо залишок на наступний блок
        leftoverLen = combinedLen - i;
        for (int j = 0; j < leftoverLen; j++)
            leftover[j] = combined[i + j];

        writeToFile(output, outputBuffer, outPos);
    }

    // Обробляємо залишок у кінці
    int outPos = 0;
    int i = 0;
    while (i < leftoverLen) {
        if (i <= leftoverLen - wordLen && isWordHere(&leftover[i], targetWord)) {
            foundCount++;
            if (mode == 'r') {
                for (int j = 0; j < replaceLen; j++)
                    outputBuffer[outPos++] = newWord[j];
            }
            i += wordLen;
        }
        else {
            outputBuffer[outPos++] = leftover[i++];
        }
    }

    writeToFile(output, outputBuffer, outPos);

    fclose(input);
    fclose(output);

    if (mode == 'r' || mode == 'd') {
        remove(fileName);
        rename("temp.tmp", fileName);
        cout << "\nFile updated.\n";
    }

    cout << "\nTotal matches found: " << foundCount << endl;
    return 0;
}
