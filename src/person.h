typedef struct {
    bool married;
    bool dead;
} Status;

typedef struct {
    const char *name;
    double age;
    const char *location;
    double body_count tag("BodyCount");
    int height;
    Status status;
} Person;
