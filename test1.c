/* A program to perform bubble sort on a
   element array of N elements where N is fed by user */

int x[10];

void bubblesort(int n, int a[]) {
  int i;
  int j;
  int k;

  i = n - 1;
  while(i > 0) {
    j = 0;
    while(j < i) {
      if(a[j] > a[j+1]) {
        k = a[j];
        a[j] = a[j+1];
        a[j+1] = k;
      }
      j = j + 1;
    }
    i = i - 1;
  }
}

void main(void) {
  int i;
  int cnt;

  cnt = input();
  i = 0;
  while (i < cnt) {
    x[i] = input();
    i = i + 1;
  }

  bubblesort(cnt, x);

  i = 0;
  while (i < cnt) {
    output(x[i]);
    i = i + 1;
  }
}
