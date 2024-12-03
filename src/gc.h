// C GC utility functions & struct.

#ifndef _DAVELIB_C_GC_
#define _DAVELIB_C_GC_

typedef struct GC	{

	void** _;
	int c;
	int v;

	int _v_;

	int _delquote_;

} GC;

typedef struct gc_report	{

	int totalDeleted;
	int v;
} gc_report;

// The int parameter is a MAX value for the number of ptrs to store. Set a reasonable 
extern volatile struct GC* initGC( int );

// Returns a reference to the Active GC data structure.
extern volatile struct GC* getGC();

// Sets the Active GC context. This is also done implicitly on a load-time call to fnc: initGC(int).
// Returns 1 on SUCCESS, 0 on FAILURE.
// It will fail if the GC* structure is not initialised properly. Use fnc: build_gc_struct( int ) to initialize a GC structure.
extern int setGC( volatile struct GC* );

// Saves a copy of heap-allocated reference, and returns a copy of the reference.
// Usage: g( ref )
extern void* g( void* );


extern char* gcchar( void* );


// Helper to free any GC contexts you have initialised with fnc:
// GC_* build_gc_struct(int);
extern int freeGC( volatile struct GC* gc );


int getRef( void* );



// self-explanatory.
// Searches the Active GC Context.
int freeRef( void* );

// This only cleans up the Active GC context. If you have made multiple GC data structures, they will need to be
// freed seperately, using fnc: freeGC(...) 
extern gc_report cleanUp(void);

// Returns 1 if there is space left in the Active GC to add a reference. 0 if there is no more space.
extern int gc_status();

// resizes either the GC reference passed as Arg 1, or the default GC object if NULL is passed.
// Arg 2 [int] is the new size.
extern int realloc_gc( volatile struct GC* _, int c );

// Will reset the Active GC structure reference to the default object created upon initializing DAVELIB GC with initGC(...).
// Returns 1 on success, or 0 on failure. It will fail if initGC(...) has not been called at least once during the process lifetime.
extern int reset_gc_ptr();

// As with initGC( int ), the int parameter is the MAX num of ptrs. Use this function for generating
// more GC data structures.
extern volatile struct GC* build_gc_struct( int );

#endif
