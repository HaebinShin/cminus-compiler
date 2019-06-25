int glob;
int arrglob[50];
int globa;

int random(int x) {
    /* user-feeding random */
    return input();
}

void shuffle(int n, int arr[]) {
    int i;
    int j;
    int t;
    i = n-1;
    while(i > 0) {
        j = random(i);
        t = arr[j];
        arr[j] = arr[i];
        arr[i] = t;
        i = i - 1;
    }
}

void main(void) {
    int i;
    int arr[10];

    i=0;
    while(i<10) {
        arr[i] = i;
        i = i + 1;
    }

    shuffle(10, arr);

    i=0;
    while(i<10) {
        output(arr[i]);
        i = i + 1;
    }
}
