namespace astral;

typedef Namespace string;
typedef Classname string;

public class ClassInfo extends Class
{
	@namespace : Namespace;
	@extends   : Class;
}

public new( namespace : Namespace&, classname : Classname&, extends : Class& )
{
	super( classname );

	@namespace = namespace.copy();
	@extends   = extends.copy();
}
