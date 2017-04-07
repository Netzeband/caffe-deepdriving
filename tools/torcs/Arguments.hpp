/**
 * Arguments.hpp
 *
 *  Created on: Mar 25, 2017
 *      Author: Andre Netzeband
 */

#ifndef ARGUMENTS_HPP_
#define ARGUMENTS_HPP_

#include <string>

/// @brief Searches for an argument in the array of application arguments.
/// @param NumberOfArguments The number of arguments in the array.
/// @param ppArguments       The argument string.
/// @param ArgumentName      The name of the argument to search for.
/// @return It returns the next argument after the argument-name. Or it returns an empty string,
///         if no argument was found with this name.
std::string getArgument(int NumberOfArguments,  char ** ppArguments, std::string ArgumentName);


#endif /* ARGUMENTS_HPP_ */
