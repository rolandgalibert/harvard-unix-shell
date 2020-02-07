struct if_struct {
        int if_result;          /* Result of corresponding if statement */
        int block;              /* THEN_BLOCK or ELSE_BLOCK */
        struct if_struct *prev; /* Pointer to subordinate if statement, if any */
};


extern struct if_struct *if_struct;
