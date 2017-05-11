/// \file Exception.h
/// \brief Definition of the RORC exceptions and related functions.
///
/// \author Pascal Boeschoten (pascal.boeschoten@cern.ch)

#ifndef ALICEO2_INCLUDE_READOUTCARD_EXCEPTION_H_
#define ALICEO2_INCLUDE_READOUTCARD_EXCEPTION_H_

#include <Common/Exceptions.h>
#include <stdexcept>
#include <boost/exception/exception.hpp>

namespace AliceO2 {
namespace roc {

//// RORC exception definitions
struct Exception : Common::Exception {};

// General exception definitions
struct PdaException : virtual Exception {};
struct MemoryMapException : virtual Exception {};
struct ParameterException : virtual Exception {};
struct ParseException : virtual Exception {};
struct InvalidParameterException : virtual ParameterException {};
struct OutOfRangeException : virtual Exception {};
struct LockException : virtual Exception {};
struct FileLockException : virtual LockException {};
struct NamedMutexLockException : virtual LockException {};
struct DeviceFinderException : virtual Exception {};
struct SharedStateException : virtual Exception {};
struct SharedObjectNotFoundException : virtual Exception {};
struct TimeoutException : virtual Exception {};

// C-RORC exception definitions
struct CrorcException : virtual Exception {};
struct CrorcArmDataGeneratorException : virtual CrorcException {};
struct CrorcArmDdlException : virtual CrorcException {};
struct CrorcInitDiuException : virtual CrorcException {};
struct CrorcCheckLinkException : virtual CrorcException {};
struct CrorcSiuCommandException : virtual CrorcException {};
struct CrorcDiuCommandException : virtual CrorcException {};
struct CrorcSiuLoopbackException : virtual CrorcException {};
struct CrorcFreeFifoException : virtual CrorcException {};
struct CrorcStartDataGeneratorException : virtual CrorcException {};
struct CrorcStartTriggerException : virtual CrorcException {};
struct CrorcStopTriggerException : virtual CrorcException {};
struct CrorcDataArrivalException : virtual CrorcException {};

// CRU exception definitions
struct CruException : virtual Exception {};

// Utility exception definition
struct UtilException : virtual boost::exception, virtual std::exception {};
struct ProgramOptionException : virtual UtilException {};
struct InvalidOptionValueException : virtual ProgramOptionException {};
struct OptionRequiredException : virtual ProgramOptionException {};

} // namespace roc
} // namespace AliceO2

#endif // ALICEO2_INCLUDE_READOUTCARD_EXCEPTION_H_
