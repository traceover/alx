#if !defined(TYPE_INFO_H_)
#define TYPE_INFO_H_

typedef size_t Type_Id;

typedef enum Type_Info_Tag {
	INTEGER,
	FLOAT,
	STRING,
	POINTER,
	ARRAY,
	DYNARRY,
} Type_Info_Tag;

typedef int Type_Info_Integer;
typedef int Type_Info_Float;
typedef int Type_Info_String;

typedef struct Type_Info_Pointer {
	Type_Id elem;
} Type_Info_Pointer;

typedef struct Type_Info_Array {
	Type_Id elem;
	size_t  count;
} Type_Info_Array;

typedef struct Type_Info_Dynarray {
	Type_Id elem;
} Type_Info_Dynarray;

typedef union Type_Info_Payload {
	Type_Info_Integer  _int;
	Type_Info_Float    _float;
	Type_Info_String   _string;
	Type_Info_Pointer  _pointer;
	Type_Info_Array    _array;
	Type_Info_Dynarray _dynarray;
} Type_Info_Payload;

typedef struct Type_Info {
	Type_Info_Payload payload;	
	Type_Info_Tag tag;
	size_t size;
	size_t align;
} Type_Info;

typedef struct Type_Table {
	Type_Info *items;
	size_t count;
	size_t capacity;
} Type_Table;

#endif // TYPE_INFO_H_
