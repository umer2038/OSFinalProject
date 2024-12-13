#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string>

using namespace std;

const int MAX_WORDS = 1000;

int countOnes(string count) {
  int total = 0;
  for (int i = 0; i < count.length(); i++) {
    if (count[i] == '1') {
      total++;
    }
  }
  return total;
}

int main() {
  string receivedWords[MAX_WORDS];
  string receivedCounts[MAX_WORDS];
  int receivedIndex;

  int fd = open("pipe1", O_RDONLY);

  read(fd, & receivedIndex, sizeof(int));

  cout << "Reducer receiving data:" << endl;

  for (int i = 0; i < receivedIndex; i++) {
    int wordLength, countLength;
    char wordBuffer[MAX_WORDS], countBuffer[MAX_WORDS];

    read(fd, & wordLength, sizeof(int));
    read(fd, wordBuffer, wordLength);
    wordBuffer[wordLength] = '\0';
    receivedWords[i] = string(wordBuffer);

    read(fd, & countLength, sizeof(int));
    read(fd, countBuffer, countLength);
    countBuffer[countLength] = '\0';
    receivedCounts[i] = string(countBuffer);

    cout << receivedWords[i] << "," << receivedCounts[i] << endl;
  }

  cout << "\nReducer output:" << endl;
  for (int i = 0; i < receivedIndex; i++) {
    string word = receivedWords[i];
    string counts = receivedCounts[i];

    int total = countOnes(counts);

    cout << word << "," << total << endl;
  }

  close(fd);

  return 0;
}
