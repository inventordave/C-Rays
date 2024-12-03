// GC_C

#include <stdlib.h>
#include <stdio.h>
#include "gc.h"

static volatile struct GC* gc;
static volatile struct GC* gc_bkp;
static int z = 0;
static int _entry = 1;

volatile struct GC* initGC( int c )	{

	if( !_entry )	{

		if( gc->_!=NULL )	{

			free( gc->_ );
		}
		free( (struct GC*) gc );
	}

	gc = build_gc_struct( c );

	if( _entry==1 )	{

		gc_bkp = (volatile struct GC*)malloc( sizeof(struct GC) );
		*gc_bkp = *gc;
		_entry=0;
	}

	return (volatile struct GC*) gc;
}
volatile struct GC* build_gc_struct( int c )	{

	volatile struct GC* gc = (volatile struct GC*)malloc( sizeof(volatile struct GC) );

	gc->_ = (void**)calloc( c,sizeof(void*) );

	if( gc->_ == NULL )
		c = 0;

	gc->c = c;
	gc->v = 0;
	gc->_v_ = 0;
	gc->_delquote_ = 0;

	return gc;
}

volatile struct GC* getGC()	{

	return gc;
}
int setGC( volatile struct GC* _ )	{

	if( _->_ != NULL )	{

		gc =  _;
		return 1;
	}

	return 0;
}

int getRef( void* ref )	{

	int k;
	for( k=0; k<gc->v; k++ )	{

		if( gc->_[k] == ref )	{
			return k;
		}
	}

	return -1;
}


int freeRef( void* ref )	{

	int k;
	for( k=0; k<gc->v; k++ )	{

		if( gc->_[k] == ref )	{

			free( gc->_[k] );
			gc->_[k] = NULL;
			gc->_v_--;
			gc->_delquote_++;
			return k;
		}
	}

	return -1;
}

int freeGC( volatile struct GC* gc )	{

	if( gc==NULL )
		return 0;

	if( gc->_!=NULL )
		freeRef( gc->_ );

	int t = gc->_v_;
	free( (struct GC*) gc );

	return t;
}

void* g( void* ref )	{

	#ifndef report
	#define report printf
	#endif
	
	loop:
	
		if( gc->c > gc->v )	{
			gc->_[gc->v++] = ref;
			gc->_v_++;
			return ref;
		}
		

	
		report( "The GC object (ref '%p') has been exhausted. Increasing to x2 size (current size = %d, new size = %d).\n", gc, gc->c, gc->c*2 );
	
		int i;
		if( (i = realloc_gc( gc, gc->c*2 ))<+1 )	{
	
			report( "Reallocation failed (status code '%d'). Exiting.\n", i );
			fflush( stdout );
			exit( 1 );
		}
	
		goto loop;

	report( "If you can see this message, the GC has not caught a reference allocation. For assistance "
					"the reference address & is '%p'.\n", ref );
	
	return ref;
}

char* gcchar( void* ref )	{

	return (char*) g( ref );
}

int gc_status()	{

	return (gc->v < gc->c);
}
int reset_gc_ptr( )	{

	if( !_entry )	{
		*gc = *gc_bkp;
		return 1;
	}

	return 0;
}
int realloc_gc( volatile struct GC* gc_, int c2 )	{

	if( gc_ == NULL )
		gc_ = gc;

	int c = gc_->c;
	int _v_ = gc->_v_;

	if( c2 == c )
		return 0;

	int t;
	if ( c2 < c )
		t = c2;
	else
		t = c;

	void** _ = (void**)malloc( sizeof(void*) * c );

	int i;

	for( i=0; i < t; i++ )
		_[i] = gc_->_[i];

	free( gc_->_ );

	gc_->_ = (void**)malloc( sizeof(void*) * c2 );

	for( i=0; i < t; i++ )
		gc_->_[i] = _[i];


	
	gc_->v = i;
	gc_->_v_ = _v_;

	
	if( i < c )	{

		free( _ );
		return -1; 
	}

	for( ; i < c2; i++ )
		gc_->_[i] = (void*)calloc( 1, sizeof(void*) );

	gc_->c = i;

	free( _ );

	return +1;
}

struct gc_report cleanUp( )	{

	int total = gc->_delquote_;
	int remaining = freeGC( gc );

	struct gc_report _;
	_.totalDeleted = total+remaining;
	_.v = remaining;
	return _;
}

