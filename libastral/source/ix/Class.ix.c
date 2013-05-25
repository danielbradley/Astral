namespace astral;

typedef Classname string;

public class Class
{
	@classname  : Classname;
}

public new( classname : Classname& )
{
	@classname = classname.copy();
}

public getClassname() : Classname&
{
	return @classname;
}
