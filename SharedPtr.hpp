#ifndef XXYYXX_SHARED_PTR_HPP
#define XXYYXX_SHARED_PTR_HPP

#include <iostream>
#include <assert.h>
#include <pthread.h>
#include <mutex>

namespace cs540
{
	std::mutex ref_lock;

	class Helper
	{
		private:	
		public:
			int ref_count;

			Helper () {}
			virtual void deleter () {};
			virtual ~ Helper ()	{}
	};

	template <typename T>
	class HelperDerived final : public Helper
	{
		private:
		public:
			T * ptr;

			HelperDerived (T * p) {ptr = p;}
			void deleter () {delete ptr;}
			~ HelperDerived () {}
	};

	template <typename T>
	class SharedPtr
	{
		private:
		public:
			T * shared_ptr;							
			Helper * helper_obj; 

			SharedPtr () 							// Default constructor
			{
				//printf("Default Const\n");
				this -> helper_obj = nullptr;
				this -> shared_ptr = nullptr;
			}			

			template <typename U> explicit			// Constructor 
			SharedPtr (U * p)
			{
				//printf("Const\n");
				if (p)
				{
					this -> helper_obj = new HelperDerived <U> (p);
					this -> helper_obj -> ref_count = 1;

					this -> shared_ptr = p;
				}
			}

			SharedPtr (const SharedPtr & p)			// Copy constructor. sp2(sp)
			{
				//printf("Copy Const\n");
				if (p.shared_ptr)
				{
					ref_lock.lock();
					p.helper_obj -> ref_count++;
					ref_lock.unlock();

					this -> shared_ptr = p.shared_ptr;
					this -> helper_obj = p.helper_obj;
				}
				else
				{
					this -> helper_obj = nullptr;
					this -> shared_ptr = nullptr;
				}
			}
			
			
			template <typename U> 
			SharedPtr (const SharedPtr <U> & p)		// Copy constructor with different type
			{
				//printf("Copy Const different type\n");
				if (p.shared_ptr)
				{
					ref_lock.lock();
					p.helper_obj -> ref_count++;
					ref_lock.unlock();

					this -> shared_ptr = p.shared_ptr;
					this -> helper_obj = p.helper_obj;
				}
				else
				{
					this -> shared_ptr = nullptr;
					this -> helper_obj = nullptr;
				}
			}

			SharedPtr (SharedPtr && p)				// Move constructor
			{
				//printf("Move Const\n");
				if (p.shared_ptr)
				{
					this -> shared_ptr = p.shared_ptr;
					this -> helper_obj = p.helper_obj;

					p.shared_ptr = nullptr;
				}
				else
				{
					this -> helper_obj = nullptr;
					this -> shared_ptr = nullptr;
				}
			}

			template <typename U> 
			SharedPtr (SharedPtr <U> && p)			// Move constructr with different type
			{
				//printf("Move Const different type\n");
				if (p.shared_ptr)
				{
					this -> shared_ptr = p.shared_ptr;
					this -> helper_obj = p.helper_obj;

					p.shared_ptr = nullptr;
				}
				else
				{
					this -> helper_obj = nullptr;
					this -> shared_ptr = nullptr;
				}
			}

			SharedPtr <T> & 						// Copy assignment operator. sp2 = sp
			operator = (const SharedPtr & p)
			{
				//printf("Copy assignment operator\n");
				ref_lock.lock();

				if (this -> shared_ptr) 
				{
					this -> helper_obj -> ref_count--;

					if (this -> helper_obj -> ref_count == 0) 
					{
						this -> helper_obj -> deleter();
						this -> shared_ptr = nullptr;

						delete this -> helper_obj;
					}
				}

				if (p.shared_ptr) 
				{
					p.helper_obj -> ref_count++;
					this -> shared_ptr = p.shared_ptr;
					this -> helper_obj = p.helper_obj;
				}
				else
				{
					this -> helper_obj = nullptr;
					this -> shared_ptr = nullptr;
				}

				ref_lock.unlock();

				return (* this);
			}
			
			template <typename U>
			SharedPtr <T> &
			operator = (const SharedPtr <U> & p)	// Copy assignment operator with different type
			{
				//printf("Copy assignment operator with different type\n");
				ref_lock.lock();

				if (this -> shared_ptr) 
				{
					this -> helper_obj -> ref_count--;

					if (this -> helper_obj -> ref_count == 0) 
					{
						this -> helper_obj -> deleter();
						this -> shared_ptr = nullptr;

						delete this -> helper_obj;
					}
				}

				if (p.shared_ptr) 
				{
					p.helper_obj -> ref_count++;

					this -> shared_ptr = p.shared_ptr;
					this -> helper_obj = p.helper_obj;
				}
				else
				{
					this -> helper_obj = nullptr;
					this -> shared_ptr = nullptr;
				}
				
				ref_lock.unlock();

				return (* this);
			}

			SharedPtr <T> &
			operator = (SharedPtr && p)				// Move assignment operator
			{
				//printf("Move assignment operator\n");
				if (p.shared_ptr)
				{
					this -> shared_ptr = p.shared_ptr;
					this -> helper_obj = p.helper_obj;

					p.shared_ptr = nullptr;
				}
				else
				{
					this -> helper_obj = nullptr;
					this -> shared_ptr = nullptr;
				}

				return (* this);
			}

			template <typename U>
			SharedPtr <T> &
			operator = (SharedPtr <U> && p)			// Move assignment operator with different type
			{
				//printf("Move assignment operator with different type\n");
				if (p.shared_ptr)
				{
					this -> shared_ptr = p.shared_ptr;
					this -> helper_obj = p.helper_obj;

					p.shared_ptr = nullptr;
				}
				else
				{
					this -> helper_obj = nullptr;
					this -> shared_ptr = nullptr;
				}
				
				return (* this);
			}

			~ SharedPtr ()							// Destructor	
			{
				//printf("Destructor ~~~\n");
				ref_lock.lock();

				if (this -> helper_obj)
				{
					this -> helper_obj -> ref_count--;

					if (this -> helper_obj -> ref_count == 0) 
					{
						this -> helper_obj -> deleter();
						delete this -> helper_obj;
					}
					else if (this -> helper_obj -> ref_count < 0) assert(false);
				}
				
				ref_lock.unlock();
			}

			void
			reset ()
			{
				ref_lock.lock();

				if (this -> helper_obj) 
				{
					this -> helper_obj -> ref_count--;

					if (this -> helper_obj -> ref_count == 0) 
					{
						this -> helper_obj -> deleter();
						delete this -> helper_obj;
					}
					else if (this -> helper_obj -> ref_count < 0) assert(false);
				}

				this -> helper_obj = nullptr;
				this -> shared_ptr = nullptr;
				
				ref_lock.unlock();
			}

			template <typename U>
			void
			reset (U * p)
			{
				ref_lock.lock();

				if (this -> helper_obj) 
				{
					this -> helper_obj -> ref_count--;

					if (this -> helper_obj -> ref_count == 0) 
					{
						this -> helper_obj -> deleter();
						delete this -> helper_obj;
					}
					else if (this -> helper_obj -> ref_count < 0) assert(false);					
				}

				if (p)
				{
					this -> helper_obj = new HelperDerived <U> (p);
					this -> helper_obj -> ref_count = 1;

					this -> shared_ptr = p;
				}
				
				ref_lock.unlock();
			}

			T *
			get () const {return this -> shared_ptr;}

			T &
			operator * () const {return * (this -> shared_ptr);}

			T * 
			operator -> () const {return this -> shared_ptr;}

			explicit 
			operator bool () const 
			{	
				if (this -> shared_ptr) return true;
				else return false;
			}
	};

	template <typename T1, typename T2>
	bool
	operator == (const SharedPtr <T1> & sp1, const SharedPtr <T2> & sp2)
	{
		if (sp1.shared_ptr == sp2.shared_ptr) return true;
		if (sp1.shared_ptr == nullptr && sp2.shared_ptr == nullptr) return true;

		return false;
	}

	template <typename T>
	bool
	operator == (const SharedPtr <T> & sp, std::nullptr_t)
	{
		if (sp.shared_ptr == nullptr) return true;
		else return false;
	}

	template <typename T>
	bool
	operator == (std::nullptr_t, const SharedPtr <T> & sp)
	{
		if (sp.shared_ptr == nullptr) return true;
		else return false;
	}

	template <typename T1, typename T2>
	bool
	operator != (const SharedPtr <T1> & sp1, const SharedPtr <T2> & sp2)
	{
		if (sp1.shared_ptr != sp2.shared_ptr) return true;
		if (sp1.shared_ptr && sp2.shared_ptr == nullptr) return true;
		if (sp2.shared_ptr && sp1.shared_ptr == nullptr) return true;

		return false;
	}

	template <typename T>
	bool
	operator != (const SharedPtr <T> & sp, std::nullptr_t)
	{
		if (sp.shared_ptr == nullptr) return false;
		else return true;
	}

	template <typename T>
	bool
	operator != (std::nullptr_t, const SharedPtr <T> & sp)
	{
		if (sp.shared_ptr == nullptr) return false;
		else return true;
	}

	template <typename T, typename U>
	SharedPtr <T>
	static_pointer_cast (const SharedPtr <U> & sp)
	{
		SharedPtr <T> s;
		s.shared_ptr = static_cast <T *> (sp.shared_ptr);
		return s;
	}

	template <typename T, typename U>
	SharedPtr <T>
	dynamic_pointer_cast (const SharedPtr <U> & sp)
	{
		SharedPtr <T> s;
		s.shared_ptr = dynamic_cast <T *> (sp.shared_ptr);
		return s;
	}
}

#endif