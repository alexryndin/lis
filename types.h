typedef struct Integer Integer;
typedef struct Integer {
    long value;
    Integer (*plus)(Integer, Integer);
} Integer;

