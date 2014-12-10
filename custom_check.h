#ifndef CUSTOM_CHECK_H_
#define CUSTOM_CHECK_H_


typedef struct custom_check {
    char *query;
    char *expected_result;
    char *relop;
} *custom_check_t;

#define CUSTOM_CHECK_QUERY(X) (X->query)
#define CUSTOM_CHECK_RESULT(X) (X->expected_result)
#define CUSTOM_CHECK_OPERATOR(X) (X->relop)

extern custom_check_t custom_check_create(char *query, char *result, char *op);
extern void custom_check_set_query(custom_check_t check, char *query);
extern void custom_check_set_result(custom_check_t check, char *result);
extern void custom_check_set_operator(custom_check_t check, char *relop);
extern void custom_check_destroy(custom_check_t check);


#endif /* CUSTOM_CHECK_H_ */
