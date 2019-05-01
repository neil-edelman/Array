/* Intended to be included by {Array.h} on {ARRAY_TYPE_FILLER}. */

/* Define macros. */
#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)



/* ARRAY_TEST must be a function that implements <PT>Action. */
static const PT_(Action) PT_(filler) = (ARRAY_TEST);



static void PT_(valid_state)(const struct T_(Array) *const a) {
	const size_t max_size = (size_t)-1 / sizeof *a->data;
	/* Null is a valid state. */
	if(!a) return;
	if(!a->data) { assert(!a->size); return; }
	assert(a->size <= a->capacity);
	assert(a->capacity < a->next_capacity
		|| (a->capacity == a->next_capacity) == max_size);
}



static void PT_(test_basic)(void) {
	struct T_(Array) a;
	T ts[5], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts, big = 1000;
	size_t i;

	printf("Test null.\n");
	errno = 0;
	T_(Array_)(0);
	T_(Array)(0);
#ifndef ARRAY_STACK /* <-- !stack */
	assert(T_(ArrayRemove)(0, 0) == 0);
#endif /* !stack --> */
	T_(ArrayClear)(0);
	assert(T_(ArrayGet)(0, 0) == 0 && T_(ArrayGet)(0, 1) == 0);
	assert(T_(ArrayPeek)(0) == 0);
	assert(T_(ArrayPop)(0) == 0);
	assert(T_(ArrayNext)(0, 0) == 0);
	assert(T_(ArrayNew)(0) == 0);
	assert(T_(ArrayUpdateNew)(0, 0) == 0
		&& T_(ArrayUpdateNew)(0, 0) == 0);
	T_(ArrayForEach)(0, 0);
	assert(!strcmp("null", T_(ArrayToString(0))));
	assert(errno == 0);
	PT_(valid_state)(0);

	printf("Test empty.\n");
	T_(Array)(&a);
	t = (T *)1;
	assert(T_(ArraySize)(&a) == 0);
#ifndef ARRAY_STACK /* <-- !stack */
	assert(T_(ArrayRemove)(&a, 0) == 0 && errno == 0);
	assert(T_(ArrayRemove)(&a, t) == 0 && errno == EDOM), errno = 0;
#endif /* !stack --> */
	assert(T_(ArrayGet)(&a, 0) == 0);
	assert(T_(ArrayPeek)(0) == 0);
	assert(T_(ArrayPop)(0) == 0);
	assert(T_(ArrayNext)(0, 0) == 0 && T_(ArrayNext)(0, t) == 0);
	T_(ArrayForEach)(&a, 0);
	assert(errno == 0);
	PT_(valid_state)(&a);

	/* @fixme valgrind is giving me grief if I don't do this? */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) PT_(filler)(t);

	printf("Test one element.\n");
	t = T_(ArrayNew)(&a); /* Add. */
	assert(t);
	t1 = T_(ArrayNext)(&a, 0);
	assert(t == t1);
	t1 = T_(ArrayNext)(&a, t);
	assert(t1 == 0);
	assert(T_(ArrayIndex)(&a, t) == 0);
	assert(T_(ArrayPeek)(&a) == t);
	assert(T_(ArrayGet(&a, 0)) == t);
	t1 = T_(ArrayPop)(&a); /* Remove. */
	assert(t1 == t);
	assert(T_(ArrayPeek)(&a) == 0);
	t = T_(ArrayNew)(&a); /* Add. */
	assert(t);
	T_(ArrayClear)(&a);
	assert(T_(ArrayPeek)(&a) == 0);
	PT_(valid_state)(&a);

	printf("Testing %lu elements.\n", (unsigned long)ts_size);
	for(i = 0; i < ts_size; i++) {
		t = T_(ArrayNew)(&a);
		assert(t);
		memcpy(t, ts + i, sizeof *t);
	}
	assert(T_(ArrayPeek)(&a));
	printf("Now: %s.\n", T_(ArrayToString)(&a));
	assert(T_(ArraySize)(&a) == ts_size);
#ifdef ARRAY_STACK /* <-- stack */
#else /* stack --><-- !stack */
	if((t = T_(ArrayGet)(&a, ts_size - 2))
		&& !T_(ArrayRemove)(&a, t)) {
		perror("Error"), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(ArrayToString)(&a));
	assert(!T_(ArrayRemove)(&a, t + 1) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;
	if((t = T_(ArrayGet)(&a, ts_size - 3))
		&& !T_(ArrayRemove)(&a, t)) {
		perror("Error"), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(ArrayToString)(&a));
	assert(!T_(ArrayRemove)(&a, t + 1) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;

	assert(a.size == ts_size - 2);
	T_(ArrayNew)(&a);
	T_(ArrayNew)(&a);
	memcpy(t + 1, ts + 3, sizeof *t * 2);
	assert(a.size == ts_size);
	PT_(valid_state)(&a);
	printf("Now: %s.\n", T_(ArrayToString)(&a));
#endif /* !stack --> */

	/* Peek/Pop. */
	t = T_(ArrayPeek)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = T_(ArrayPop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = T_(ArrayPop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 2, sizeof *t));
	T_(ArrayClear)(&a);

	/* Big. */
	for(i = 0; i < big; i++) {
		t = T_(ArrayNew)(&a);
		assert(t);
		PT_(filler)(t);
	}
	printf("%s.\n", T_(ArrayToString)(&a));
	PT_(valid_state)(&a);
	for(i = 0, t = 0; (t = T_(ArrayNext)(&a, t)); i++);
	assert(a.size == i);
	PT_(valid_state)(&a);

	printf("Clear:\n");
	T_(ArrayClear)(&a);
	printf("%s.\n", T_(ArrayToString)(&a));
	assert(T_(ArrayPeek)(&a) == 0);
	printf("Destructor:\n");
	T_(Array_)(&a);
	assert(T_(ArrayPeek)(&a) == 0);
	PT_(valid_state)(&a);
}

static void PT_(test_random)(void) {
	struct T_(Array) a;
	size_t i, size = 0;
	const size_t mult = 1; /* For long tests. */
	/* Random. */
	T_(Array)(&a);
	/* This parameter controls how many iterations. */
	i = 1000 * mult;
	while(i--) {
		T *data;
		char str[12];
		double r = rand() / (RAND_MAX + 1.0);
		/* This parameter controls how big the pool wants to be. */
		if(r > size / (100.0 * mult)) {
			if(!(data = T_(ArrayNew)(&a))) {
				perror("Error"), assert(0);
				return;
			}
			size++;
			PT_(filler)(data);
			PT_(to_string)(data, &str);
			printf("Created %s.\n", str);
		} else {
#ifdef ARRAY_STACK /* <-- stack */
			double t = 1.0;
#else /* stack --><-- !stack */
			double t = 0.5;
#endif /* !stack --> */
			r = rand() / (RAND_MAX + 1.0);
			if(r < t) {
				data = T_(ArrayPeek)(&a);
				assert(data);
				PT_(to_string)(data, &str);
				printf("Popping %s.\n", str);
				assert(data == T_(ArrayPop)(&a));
			} else {
#ifndef ARRAY_STACK /* <-- !stack */
				size_t idx = rand() / (RAND_MAX + 1.0) * size;
				if(!(data = T_(ArrayGet)(&a, idx))) continue;
				PT_(to_string)(data, &str);
				printf("Removing %s at %lu.\n", str, (unsigned long)idx);
				{
					const int ret = T_(ArrayRemove)(&a, data);
					assert(ret || (perror("Removing"), 0));
				}
#endif /* !stack --> */
			}
			size--;
		}
		printf("%s.\n", T_(ArrayToString)(&a));
		PT_(valid_state)(&a);
	}
	T_(Array_)(&a);
}

/** The list will be tested on stdout. */
static void T_(ArrayTest)(void) {
	printf("Array<" QUOTE(ARRAY_NAME) ">: of type <" QUOTE(ARRAY_TYPE)
		"> was created using: "
#ifdef ARRAY_STACK
		"ARRAY_STACK; "
#endif
#ifdef ARRAY_TAIL_DELETE
		"ARRAY_TAIL_DELETE; "
#endif
#ifdef ARRAY_MIGRATE_EACH
		"ARRAY_MIGRATE_EACH<" QUOTE(ARRAY_MIGRATE_EACH) ">; "
#endif
#ifdef ARRAY_MIGRATE_UPDATE
		"ARRAY_MIGRATE_UPDATE<" QUOTE(ARRAY_MIGRATE_UPDATE) ">; "
#endif		   
#ifdef ARRAY_TO_STRING
		"ARRAY_TO_STRING<" QUOTE(ARRAY_TO_STRING) ">; "
#endif
#ifdef ARRAY_TEST
		"ARRAY_TEST<" QUOTE(ARRAY_TEST) ">; "
#endif
		"testing:\n");
	PT_(test_basic)();
	PT_(test_random)();
	fprintf(stderr, "Done tests of Array<" T_NAME ">.\n\n");
}

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_