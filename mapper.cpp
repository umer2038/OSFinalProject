#include <iostream>
#include <string>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

const int MAX_WORDS = 1000;
string keys[MAX_WORDS];
int currentIndex = 0;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

string shuffleKeys[MAX_WORDS];
string shuffleCounts[MAX_WORDS];
int shuffleIndex = 0;

struct ThreadData {
  string word;
};

void shuffle() {

  mkfifo("pipe1", 0666);

  int fd = open("pipe1", O_WRONLY);
  if (fd == -1) {
    cerr << "Failed to open pipe for writing" << endl;
    return;
  }

  cout << "\nShuffle output:" << endl;

  string processedWords[MAX_WORDS];
  string processedCounts[MAX_WORDS];
  int processedIndex = 0;

  for (int i = 0; i < currentIndex; i++) {
    string currentWord = keys[i];
    bool found = false;

    for (int j = 0; j < processedIndex; j++) {
      if (processedWords[j] == currentWord) {
        processedCounts[j] += ",1";
        found = true;
        break;
      }
    }

    if (!found) {
      processedWords[processedIndex] = currentWord;
      processedCounts[processedIndex] = "1";
      processedIndex++;
    }
  }

  write(fd, & processedIndex, sizeof(int));

  for (int i = 0; i < processedIndex; i++) {
    int wordLength = processedWords[i].length();
    int countLength = processedCounts[i].length();

    write(fd, & wordLength, sizeof(int));
    write(fd, processedWords[i].c_str(), wordLength);

    write(fd, & countLength, sizeof(int));
    write(fd, processedCounts[i].c_str(), countLength);

    cout << processedWords[i] << "," << processedCounts[i] << endl;
  }

  close(fd);
}

void * mapper(void * arg) {
  ThreadData * threadData = (ThreadData * ) arg;
  string word = threadData -> word;

  string wordToStore = word;
  int myIndex;

  pthread_mutex_lock( & lock1);
  myIndex = currentIndex;
  keys[currentIndex] = wordToStore;
  currentIndex++;
  pthread_mutex_unlock( & lock1);

  pthread_mutex_lock( & print_lock);
  cout << wordToStore << "," << "1" << endl;
  pthread_mutex_unlock( & print_lock);

  delete threadData;
  return NULL;
}

int main() {
  string sentence;

  cout << "Enter a sentence: ";
  getline(cin, sentence);

  int wordCount = 1;
  for (int i = 0; i < sentence.length(); i++) {
    if (sentence[i] == ' ' && sentence[i + 1] != ' ') {
      wordCount++;
    }
  }

  string * words = new string[wordCount];

  int arrayIndex = 0;
  string currentWord = "";

  for (int i = 0; i < sentence.length(); i++) {
    if (sentence[i] != ' ') {
      currentWord += sentence[i];
    }
    if ((sentence[i] == ' ' || i == sentence.length() - 1) && !currentWord.empty()) {
      words[arrayIndex] = currentWord;
      arrayIndex++;
      currentWord = "";
    }
  }

  pthread_t * threads = new pthread_t[wordCount];

  cout << "\nMapper output:" << endl;
  for (int i = 0; i < wordCount; i++) {
    ThreadData * threadData = new ThreadData;
    threadData -> word = words[i];

    if (pthread_create( & threads[i], NULL, mapper, (void * ) threadData) != 0) {
      cerr << "Failed to create thread " << i << endl;
      return 1;
    }
  }

  for (int i = 0; i < wordCount; i++) {
    pthread_join(threads[i], NULL);
  }

  shuffle();

  delete[] words;
  delete[] threads;
  pthread_mutex_destroy( & lock1);
  pthread_mutex_destroy( & print_lock);

  return 0;
}
