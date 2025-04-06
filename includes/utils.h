/// <summary>
/// A two-dimensional vector of floats
/// </summary>
typedef struct
{
	float x;
	float y;
} Vec2;

/// <summary>
/// A two-dimensional vector of integers
/// </summary>
typedef struct
{
	int x;
	int y;
} iVec2;

/// <summary>
/// A three-dimensional vector of floats
/// </summary>
typedef struct
{
	union
	{
		float x;
		float r;
	};
	union
	{
		float y;
		float g;
	};
	union
	{
		float z;
		float b;
	};
} Vec3;

/// <summary>
/// A three-dimensional vector of integers
/// </summary>
typedef struct
{
	union
	{
		int x;
		int r;
	};
	union
	{
		int y;
		int g;
	};
	union
	{
		int z;
		int b;
	};
} iVec3;
