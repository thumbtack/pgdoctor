#ifndef HEALTH_CHECK_H
#define HEALTH_CHECK_H


typedef struct health_check {
    char *query;
    char *expected_result;
    char *relop;
} *health_check_t;

#define HEALTH_CHECK_QUERY(X) (X->query)
#define HEALTH_CHECK_RESULT(X) (X->expected_result)
#define HEALTH_CHECK_OPERATOR(X) (X->relop)

extern health_check_t health_check_create(char *query, char *result, char *operator);
extern void health_check_set_query(health_check_t check, char *query);
extern void health_check_set_result(health_check_t check, char *expected_result);
extern void health_check_set_operator(health_check_t check, char *relop);
extern void health_check_destroy(health_check_t check);


#endif
