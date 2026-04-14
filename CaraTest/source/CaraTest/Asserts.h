#pragma once

#include <CaraTest/API.h>
#include <CaraTest/Stringify.h>
#include <CaraTest/StringifyForQt.h>
#include <CaraTest/HelperExceptions.h>
#include <CaraTest/File.h>
#include <CaraTest/Tests.h>
#include <filesystem>

namespace CaraTest
{
    [[noreturn]] CARATEST_API void fail(const std::source_location& location = std::source_location::current());
    [[noreturn]] CARATEST_API void skip(const std::source_location& location = std::source_location::current());
    CARATEST_API void isTrue(bool value, const std::source_location& location = std::source_location::current());
    CARATEST_API void isFalse(bool value, const std::source_location& location = std::source_location::current());

    template<class T1, class T2>
    void areEqual(
        T1&& expectedValue, 
        T2&& actualValue, 
        const std::source_location& location = std::source_location::current())
    {
        if (expectedValue != actualValue)
        {
            HANDLE_CARATEST_FAILURE();
            throw ValueMismatchTestException(
                stringifyAndMaybeQuote(expectedValue),
                stringifyAndMaybeQuote(actualValue),
                location,
                ValueMismatchTestException::OutputMode::Diff
            );
        }
    }

    template<class T1, class T2>
    void contains(
        T1&& haystackValue,
        T2&& needleValue,
        const std::source_location& location = std::source_location::current())
    {
        const auto stringifiedHaystack = stringify(haystackValue);
        const auto stringifiedNeedle = stringify(needleValue);

        if (stringifiedHaystack.find(stringifiedNeedle) == std::string::npos)
        {
            HANDLE_CARATEST_FAILURE();
            throw ValueMismatchTestException(
                stringifyAndMaybeQuote(stringifiedNeedle),
                stringifyAndMaybeQuote(stringifiedHaystack),
                location,
                ValueMismatchTestException::OutputMode::All
            );
        }
    }

    template<class T1>
    void equalsFile(
        const std::filesystem::path& expectedContentFilePath, 
        T1&& actualValue, 
        const std::source_location& location = std::source_location::current())
    {
        const auto stringifiedActualValue = stringify(actualValue);

        if (!std::filesystem::exists(expectedContentFilePath))
        {
            const auto snapshotFilePath = expectedContentFilePath.string() + ".snapshot";
            File::writeContent(snapshotFilePath, stringifiedActualValue);

            HANDLE_CARATEST_FAILURE();
            throw SnapshotCreatedTestException(
                std::string(), 
                stringifyAndMaybeQuote(actualValue), 
                snapshotFilePath, 
                location
            );
        }

        const auto optionalFileContent = File::readContent(expectedContentFilePath);
        if (!optionalFileContent.has_value())
        {
            fail(location);
        }

        const auto fileContent = optionalFileContent.value();
        const auto normalizedFileContent = normalizeNewlines(fileContent);
        const auto normalizedActualValue = normalizeNewlines(stringifiedActualValue);
        if (normalizedActualValue != normalizedFileContent)
        {
            const auto snapshotFilePath = expectedContentFilePath.string() + ".snapshot";
            File::writeContent(snapshotFilePath, stringifiedActualValue);

            HANDLE_CARATEST_FAILURE();
            throw SnapshotCreatedTestException(
                stringifyAndMaybeQuote(normalizedFileContent), 
                stringifyAndMaybeQuote(normalizedActualValue), 
                snapshotFilePath, 
                location
            );
        }
    }
}

#define CARATEST_DEBUG_BREAK_ON_PARTIAL_MATCH(firstValue, secondValue)                                              \
    do {                                                                                                            \
        if ((firstValue) == (secondValue)) {                                                                        \
            TRIGGER_DEBUG_BREAK();                                                                                  \
        } else  {                                                                                                   \
            const auto _firstStringified = stringifyAndMaybeQuote(firstValue);                                      \
            const auto _secondStringified = stringifyAndMaybeQuote(secondValue);                                    \
            if (_firstStringified.find(_secondStringified) != std::string::npos ||                                  \
                _secondStringified.find(_firstStringified) != std::string::npos) {                                  \
                TRIGGER_DEBUG_BREAK();                                                                              \
            }                                                                                                       \
        }                                                                                                           \
    } while(0)
