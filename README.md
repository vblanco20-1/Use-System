Use-System
==========

Very simple use system for use in both c++ and blueprints, on top of the example FPS template


How to use it
=============

The system works both in c++ and blueprints, in c++, you just need to inherit from IUsable interface in your class and implement the OnUsed() and OnFocus() functions.
It can also added to ANY blueprint you want, doesnt matter wich class it is, just go to blueprint properties, and add the Usable interface, you will be able now to add the OnUsed and OnFocus events, and use those as you want.
In the case you want both c++ and blueprints, implementing your own OnUsed in c++, then you should call the blueprint version, as if you change the events in c++, it wont call the blueprint by default, so just call the function
	
	
	IUsable::Execute_BTOnUsed(this, user); 
	or 
	IUsable::Execute_BTOnFocus(this, user);
	
in your respective OnUsed and OnFocus functions, so it calls the blueprint event, ending somewhat like this
	
	void Ayourclass::OnUsed(AController*user)
	{
	//custom code here
	
	
	// call blueprint event
	IUsable::Execute_BTOnUsed(this, user); 
	}

Here you can see an example of how to use it in blueprints https://www.youtube.com/watch?v=flISoAMGqAQ


You are free to do whatever you want with the code.
