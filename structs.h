#ifndef STRUCTS
#define STRUCTS

// stucts.h

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ABS(a) (((a)<0)?(-a):(a))

struct coord {
    int y;
    int x;
};

struct alignment {
    int y;
    int x;
};

struct ship {
	int id;
	int length;
    int health;
	struct coord bow_coord;
	struct coord stern_coord;
    struct alignment align;
};

struct move {
    struct coord old;
    struct coord new;
    int type;
};


#endif
