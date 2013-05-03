#include <string>
#include <exception>

#ifndef ICM_ORMEXCEPTION_HPP
#define ICM_ORMEXCEPTION_HPP
namespace ORawrM
{
	class Error : public std::exception
	{
	public:
		Error(const std::string& what, size_t errorCode);
		~Error() throw();

		//This value is adapter specific, and may hold no meaningful value.
		size_t code() const;
		const char * what() const throw();
	private:
		std::string errorMessage;
		size_t errorCode;
	};
}
#endif
