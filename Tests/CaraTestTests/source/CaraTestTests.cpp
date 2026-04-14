#include <CaraTest.h>

#include <filesystem>
#include <random>
#include <sstream>

using namespace CaraTest;

namespace Simple
{
    static void TestCanExecute()
    {
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};
        int i = 0;
        suite.add(std::string(), [&i]() {i++; });

        const auto result = runner.run(suite);

        CaraTest::areEqual(i, 1);
    }

    static void TestCanPass()
    {
        int expectedPasses = 1;
        int expectedFails = 0;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                // do nothing
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestCanFail()
    {
        int expectedPasses = 0;
        int expectedFails = 1;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                CaraTest::fail();
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestCanBeSkipped()
    {
        int expectedPasses = 0;
        int expectedFails = 0;
        int expectedSkips = 1;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                CaraTest::skip();
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestSuitesReturnsAddedTests()
    {
        auto expectedTestCount = 2;
        TestSuite suite{};
        suite.add(std::string(), []()
            {
                CaraTest::skip();
            });
        suite.add(std::string(), []()
            {
                CaraTest::fail();
            });

        CaraTest::areEqual(expectedTestCount, (int)suite.tests().size());
    }

    static void TestContainsCallingTestName()
    {
        std::string expectedName = std::string("TestContainsCallingTestName");
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};
        suite.add(std::string("TestContainsCallingTestName"), []()
            {
                CaraTest::skip();
            });

        auto& test = suite.tests().at(0);

        CaraTest::areEqual(expectedName, test->name());
    }

    static void TestIsTrueWhenTrue()
    {
        int expectedPasses = 1;
        int expectedFails = 0;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                auto value = true;
                CaraTest::isTrue(value);
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestIsTrueWhenFalse()
    {
        int expectedPasses = 0;
        int expectedFails = 1;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                auto value = false;
                CaraTest::isTrue(value);
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestIsFalseWhenFalse()
    {
        int expectedPasses = 1;
        int expectedFails = 0;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                auto value = false;
                CaraTest::isFalse(value);
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestIsFalseWhenTrue()
    {
        int expectedPasses = 0;
        int expectedFails = 1;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                auto value = true;
                CaraTest::isFalse(value);
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestAreEqualWhenTrueAndTrue()
    {
        int expectedPasses = 1;
        int expectedFails = 0;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                auto expectedValue = true;
                auto actualValue = true;
                CaraTest::areEqual(expectedValue, actualValue);
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestAreEqualWhenFalseAndFalse()
    {
        int expectedPasses = 1;
        int expectedFails = 0;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                auto expectedValue = false;
                auto actualValue = false;
                CaraTest::areEqual(expectedValue, actualValue);
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestAreEqualWhenTrueAndFalse()
    {
        int expectedPasses = 0;
        int expectedFails = 1;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                auto expectedValue = true;
                auto actualValue = false;
                CaraTest::areEqual(expectedValue, actualValue);
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestAreEqualWhenZeroAndZero()
    {
        int expectedPasses = 1;
        int expectedFails = 0;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                auto expectedValue = 0;
                auto actualValue = 0;
                CaraTest::areEqual(expectedValue, actualValue);
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestAreEqualWhenOneAndOne()
    {
        int expectedPasses = 1;
        int expectedFails = 0;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                auto expectedValue = 1;
                auto actualValue = 1;
                CaraTest::areEqual(expectedValue, actualValue);
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestAreEqualWhenOneAndZero()
    {
        int expectedPasses = 0;
        int expectedFails = 1;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                auto expectedValue = 1;
                auto actualValue = 0;
                CaraTest::areEqual(expectedValue, actualValue);
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestAreEqualWhenEmptyStringAndEmptyString()
    {
        int expectedPasses = 1;
        int expectedFails = 0;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                auto expectedValue = std::string();
                auto actualValue = std::string();
                CaraTest::areEqual(expectedValue, actualValue);
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestAreEqualWhenStringAndSameString()
    {
        int expectedPasses = 1;
        int expectedFails = 0;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                auto expectedValue = std::string("string");
                auto actualValue = std::string("string");
                CaraTest::areEqual(expectedValue, actualValue);
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestAreEqualWhenStringAndDifferentString()
    {
        int expectedPasses = 0;
        int expectedFails = 1;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                auto expectedValue = std::string("string");
                auto actualValue = std::string("other");
                CaraTest::areEqual(expectedValue, actualValue);
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestContainsWhenSubstringExists()
    {
        int expectedPasses = 1;
        int expectedFails = 0;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                CaraTest::contains(std::string("str"), std::string("string"));
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestContainsWhenSubstringDoesNotExist()
    {
        int expectedPasses = 0;
        int expectedFails = 1;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), []()
            {
                CaraTest::contains(std::string("other"), std::string("string"));
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestRunnerExecutesTests()
    {
        int expectedPasses = 1;
        int expectedFails = 1;
        int expectedSkips = 1;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};
        suite.add(std::string(), []()
            {
            });
        suite.add(std::string(), []()
            {
                CaraTest::skip();
            });
        suite.add(std::string(), []()
            {
                CaraTest::fail();
            });

        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }
}

namespace Parameterized
{
    static void TestCanExecuteWithParameters(bool data)
    {
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};
        int i = 0;
        suite.add(std::string(), [&i]() {i++; });

        const auto result = runner.run(suite);

        CaraTest::areEqual(i, 1);
    }

    static std::vector<std::tuple<bool>> TestCanExecuteWithParameters_Data()
    {
        return { std::make_tuple(true), std::make_tuple(false) };
    }

    static void TestWithTwoEmptySubTestsHasTwoPasses()
    {
        int expectedPasses = 2;
        int expectedFails = 0;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), [](bool value)
            {
                // do nothing
            }, 
            []() 
            { 
                return std::vector{ std::make_tuple(true), std::make_tuple(false) };
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestWithSkipOnTrueInThreeSubTestsHasTwoPassesAndOneSkip()
    {
        int expectedPasses = 2;
        int expectedFails = 0;
        int expectedSkips = 1;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), [](bool value)
            {
                if (value)
                    CaraTest::skip();
            },
            []()
            {
                return std::vector{ std::make_tuple(true), std::make_tuple(false), std::make_tuple(false) };
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestWithFailInTwoSubTestsHasTwoFails()
    {
        int expectedPasses = 0;
        int expectedFails = 2;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), [](bool value)
            {
                CaraTest::fail();
            },
            []()
            {
                return std::vector{ std::make_tuple(false), std::make_tuple(false) };
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestWithIsTrueCheckInTwoSubTestsHasOnePassAndOneFail()
    {
        int expectedPasses = 1;
        int expectedFails = 1;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), [](bool value)
            {
                CaraTest::isTrue(value);
            },
            []()
            {
                return std::vector{ std::make_tuple(true), std::make_tuple(false) };
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestWithSkipOnTrueAndFailHasOneSkipAndTwoFails()
    {
        int expectedPasses = 0;
        int expectedFails = 2;
        int expectedSkips = 1;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), [](bool value)
            {
                if (value)
                    CaraTest::skip();

                CaraTest::fail();
            },
            []()
            {
                return std::vector{ std::make_tuple(false), std::make_tuple(true), std::make_tuple(false) };
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestWithSkipInTwoSubTestsHasTwoSkips()
    {
        int expectedPasses = 0;
        int expectedFails = 0;
        int expectedSkips = 2;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), [](bool value)
            {
                CaraTest::skip();
            },
            []()
            {
                return std::vector{ std::make_tuple(true), std::make_tuple(false) };
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }

    static void TestWithoutDataDontExecute()
    {
        int expectedPasses = 0;
        int expectedFails = 0;
        int expectedSkips = 0;
        TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
        TestSuite suite{};

        suite.add(std::string(), [](bool value)
            {
                // do nothing
            },
            []()
            {
                return std::vector<std::tuple<bool>>{};
            });
        const auto result = runner.run(suite);

        CaraTest::areEqual(expectedPasses, suite.passedTests());
        CaraTest::areEqual(expectedFails, suite.failedTests());
        CaraTest::areEqual(expectedSkips, suite.skippedTests());
    }
}

static std::filesystem::path GenerateRandomFilePath()
{
    // Generate a random number using the standard library
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, std::numeric_limits<int>::max());
    const auto value = distribution(generator);

    // Construct the file path using std::filesystem
    const auto tempPath = std::filesystem::temp_directory_path();
    std::ostringstream fileNameStream;
    fileNameStream << "test_file_" << value << ".test";
    return tempPath / fileNameStream.str();
}

static void EqualsFileCreatesSnapshotWhenFileDoesNotExist()
{
    int expectedPasses = 0;
    int expectedFails = 1;
    int expectedSkips = 0;
    TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
    TestSuite suite{};

    const auto testFilePath = GenerateRandomFilePath();

    suite.add(std::string(), [=]()
        {
            CaraTest::equalsFile(testFilePath, std::string("TestValue"));
        });
    const auto result = runner.run(suite);

    const auto snapshotFilePath = testFilePath.string() + ".snapshot";
    const auto fileExists = std::filesystem::exists(snapshotFilePath);
    bool wasRemoved = false;
    if (fileExists)
    {
        wasRemoved = std::filesystem::remove(snapshotFilePath);
    }

    CaraTest::isTrue(fileExists);
    CaraTest::isTrue(wasRemoved);
    CaraTest::areEqual(expectedPasses, suite.passedTests());
    CaraTest::areEqual(expectedFails, suite.failedTests());
    CaraTest::areEqual(expectedSkips, suite.skippedTests());
}

static void EqualsFileDoesntCreateSnapshotWhenValuesareEqual()
{
    int expectedPasses = 1;
    int expectedFails = 0;
    int expectedSkips = 0;
    std::string expectedValue("TestValue");

    TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
    TestSuite suite{};

    const auto testFilePath = GenerateRandomFilePath();
    CaraTest::File::writeContent(testFilePath, expectedValue);

    suite.add(std::string(), [=]()
        {
            CaraTest::equalsFile(testFilePath, expectedValue);
        });
    const auto result = runner.run(suite);

    const auto snapshotFilePath = testFilePath.string() + ".snapshot";
    const auto fileExists = std::filesystem::exists(snapshotFilePath);
    bool wasRemoved = false;
    if (fileExists)
    {
        wasRemoved = std::filesystem::remove(snapshotFilePath);
    }

    CaraTest::isFalse(fileExists);
    CaraTest::isFalse(wasRemoved);
    CaraTest::areEqual(expectedPasses, suite.passedTests());
    CaraTest::areEqual(expectedFails, suite.failedTests());
    CaraTest::areEqual(expectedSkips, suite.skippedTests());
}

static void EqualsFileCreateSnapshotWhenValuesAreNotEqual()
{
    int expectedPasses = 0;
    int expectedFails = 1;
    int expectedSkips = 0;
    std::string expectedValue("TestValue");

    TestRunner runner{ 0, nullptr, TestRunner::OutputMode::None };
    TestSuite suite{};

    const auto testFilePath = GenerateRandomFilePath();
    CaraTest::File::writeContent(testFilePath, expectedValue);

    suite.add(std::string(), [=]()
        {
            CaraTest::equalsFile(testFilePath, std::string("Some other Value"));
        });
    const auto result = runner.run(suite);

    const auto snapshotFilePath = testFilePath.string() + ".snapshot";
    const auto fileExists = std::filesystem::exists(snapshotFilePath);
    bool wasRemoved = false;
    if (fileExists)
    {
        wasRemoved = std::filesystem::remove(snapshotFilePath);
    }

    CaraTest::isTrue(fileExists);
    CaraTest::isTrue(wasRemoved);
    CaraTest::areEqual(expectedPasses, suite.passedTests());
    CaraTest::areEqual(expectedFails, suite.failedTests());
    CaraTest::areEqual(expectedSkips, suite.skippedTests());
}



using namespace Simple;
auto simple = CaraTest::getSuite("SimpleTests");
auto simpleTest01 = simple->add("TestCanExecute", TestCanExecute);
auto simpleTest02 = simple->add("TestCanPass", TestCanPass);
auto simpleTest03 = simple->add("TestCanFail", TestCanFail);
auto simpleTest04 = simple->add("TestCanBeSkipped", TestCanBeSkipped);
auto simpleTest05 = simple->add("TestSuitesReturnsAddedTests", TestSuitesReturnsAddedTests);
auto simpleTest06 = simple->add("TestContainsCallingTestName", TestContainsCallingTestName);
auto simpleTest07 = simple->add("TestIsTrueWhenTrue", TestIsTrueWhenTrue);
auto simpleTest08 = simple->add("TestIsTrueWhenFalse", TestIsTrueWhenFalse);
auto simpleTest09 = simple->add("TestIsFalseWhenFalse", TestIsFalseWhenFalse);
auto simpleTest10 = simple->add("TestIsFalseWhenTrue", TestIsFalseWhenTrue);
auto simpleTest11 = simple->add("TestAreEqualWhenTrueAndTrue", TestAreEqualWhenTrueAndTrue);
auto simpleTest12 = simple->add("TestAreEqualWhenFalseAndFalse", TestAreEqualWhenFalseAndFalse);
auto simpleTest13 = simple->add("TestAreEqualWhenTrueAndFalse", TestAreEqualWhenTrueAndFalse);
auto simpleTest14 = simple->add("TestAreEqualWhenZeroAndZero", TestAreEqualWhenZeroAndZero);
auto simpleTest15 = simple->add("TestAreEqualWhenOneAndOne", TestAreEqualWhenOneAndOne);
auto simpleTest16 = simple->add("TestAreEqualWhenOneAndZero", TestAreEqualWhenOneAndZero);
auto simpleTest17 = simple->add("TestAreEqualWhenEmptyStringAndEmptyString", TestAreEqualWhenEmptyStringAndEmptyString);
auto simpleTest18 = simple->add("TestAreEqualWhenStringAndSameString", TestAreEqualWhenStringAndSameString);
auto simpleTest19 = simple->add("TestAreEqualWhenStringAndDifferentString", TestAreEqualWhenStringAndDifferentString);
auto simpleTest20 = simple->add("TestRunnerExecutesTests", TestRunnerExecutesTests);
auto simpleTest21 = simple->add("TestCanExecute", TestCanExecute);
auto simpleTest22 = simple->add("TestCanPass", TestCanPass);
auto simpleTest23 = simple->add("TestCanFail", TestCanFail);
auto simpleTest24 = simple->add("TestCanBeSkipped", TestCanBeSkipped);
auto simpleTest25 = simple->add("TestSuitesReturnsAddedTests", TestSuitesReturnsAddedTests);
auto simpleTest26 = simple->add("TestContainsCallingTestName", TestContainsCallingTestName);
auto simpleTest27 = simple->add("TestIsTrueWhenTrue", TestIsTrueWhenTrue);
auto simpleTest28 = simple->add("TestIsTrueWhenFalse", TestIsTrueWhenFalse);
auto simpleTest29 = simple->add("TestIsFalseWhenFalse", TestIsFalseWhenFalse);
auto simpleTest30 = simple->add("TestIsFalseWhenTrue", TestIsFalseWhenTrue);
auto simpleTest31 = simple->add("TestAreEqualWhenTrueAndTrue", TestAreEqualWhenTrueAndTrue);
auto simpleTest32 = simple->add("TestAreEqualWhenFalseAndFalse", TestAreEqualWhenFalseAndFalse);
auto simpleTest33 = simple->add("TestAreEqualWhenTrueAndFalse", TestAreEqualWhenTrueAndFalse);
auto simpleTest34 = simple->add("TestAreEqualWhenZeroAndZero", TestAreEqualWhenZeroAndZero);
auto simpleTest35 = simple->add("TestAreEqualWhenOneAndOne", TestAreEqualWhenOneAndOne);
auto simpleTest36 = simple->add("TestAreEqualWhenOneAndZero", TestAreEqualWhenOneAndZero);
auto simpleTest37 = simple->add("TestAreEqualWhenEmptyStringAndEmptyString", TestAreEqualWhenEmptyStringAndEmptyString);
auto simpleTest38 = simple->add("TestAreEqualWhenStringAndSameString", TestAreEqualWhenStringAndSameString);
auto simpleTest39 = simple->add("TestAreEqualWhenStringAndDifferentString", TestAreEqualWhenStringAndDifferentString);
auto simpleTest40 = simple->add("TestRunnerExecutesTests", TestRunnerExecutesTests);
auto simpleTest41 = simple->add("TestCanExecute", TestCanExecute);
auto simpleTest42 = simple->add("TestCanPass", TestCanPass);
auto simpleTest43 = simple->add("TestCanFail", TestCanFail);
auto simpleTest44 = simple->add("TestCanBeSkipped", TestCanBeSkipped);
auto simpleTest45 = simple->add("TestSuitesReturnsAddedTests", TestSuitesReturnsAddedTests);
auto simpleTest46 = simple->add("TestContainsCallingTestName", TestContainsCallingTestName);
auto simpleTest47 = simple->add("TestIsTrueWhenTrue", TestIsTrueWhenTrue);
auto simpleTest48 = simple->add("TestIsTrueWhenFalse", TestIsTrueWhenFalse);
auto simpleTest49 = simple->add("TestIsFalseWhenFalse", TestIsFalseWhenFalse);
auto simpleTest50 = simple->add("TestIsFalseWhenTrue", TestIsFalseWhenTrue);
auto simpleTest51 = simple->add("TestAreEqualWhenTrueAndTrue", TestAreEqualWhenTrueAndTrue);
auto simpleTest52 = simple->add("TestAreEqualWhenFalseAndFalse", TestAreEqualWhenFalseAndFalse);
auto simpleTest53 = simple->add("TestAreEqualWhenTrueAndFalse", TestAreEqualWhenTrueAndFalse);
auto simpleTest54 = simple->add("TestAreEqualWhenZeroAndZero", TestAreEqualWhenZeroAndZero);
auto simpleTest55 = simple->add("TestAreEqualWhenOneAndOne", TestAreEqualWhenOneAndOne);
auto simpleTest56 = simple->add("TestAreEqualWhenOneAndZero", TestAreEqualWhenOneAndZero);
auto simpleTest57 = simple->add("TestAreEqualWhenEmptyStringAndEmptyString", TestAreEqualWhenEmptyStringAndEmptyString);
auto simpleTest58 = simple->add("TestAreEqualWhenStringAndSameString", TestAreEqualWhenStringAndSameString);
auto simpleTest59 = simple->add("TestAreEqualWhenStringAndDifferentString", TestAreEqualWhenStringAndDifferentString);
auto simpleTest60 = simple->add("TestRunnerExecutesTests", TestRunnerExecutesTests);
auto simpleTest61 = simple->add("TestCanExecute", TestCanExecute);
auto simpleTest62 = simple->add("TestCanPass", TestCanPass);
auto simpleTest63 = simple->add("TestCanFail", TestCanFail);
auto simpleTest64 = simple->add("TestCanBeSkipped", TestCanBeSkipped);
auto simpleTest65 = simple->add("TestSuitesReturnsAddedTests", TestSuitesReturnsAddedTests);
auto simpleTest66 = simple->add("TestContainsCallingTestName", TestContainsCallingTestName);
auto simpleTest67 = simple->add("TestIsTrueWhenTrue", TestIsTrueWhenTrue);
auto simpleTest68 = simple->add("TestIsTrueWhenFalse", TestIsTrueWhenFalse);
auto simpleTest69 = simple->add("TestIsFalseWhenFalse", TestIsFalseWhenFalse);
auto simpleTest70 = simple->add("TestIsFalseWhenTrue", TestIsFalseWhenTrue);
auto simpleTest71 = simple->add("TestAreEqualWhenTrueAndTrue", TestAreEqualWhenTrueAndTrue);
auto simpleTest72 = simple->add("TestAreEqualWhenFalseAndFalse", TestAreEqualWhenFalseAndFalse);
auto simpleTest73 = simple->add("TestAreEqualWhenTrueAndFalse", TestAreEqualWhenTrueAndFalse);
auto simpleTest74 = simple->add("TestAreEqualWhenZeroAndZero", TestAreEqualWhenZeroAndZero);
auto simpleTest75 = simple->add("TestAreEqualWhenOneAndOne", TestAreEqualWhenOneAndOne);
auto simpleTest76 = simple->add("TestAreEqualWhenOneAndZero", TestAreEqualWhenOneAndZero);
auto simpleTest77 = simple->add("TestAreEqualWhenEmptyStringAndEmptyString", TestAreEqualWhenEmptyStringAndEmptyString);
auto simpleTest78 = simple->add("TestAreEqualWhenStringAndSameString", TestAreEqualWhenStringAndSameString);
auto simpleTest79 = simple->add("TestAreEqualWhenStringAndDifferentString", TestAreEqualWhenStringAndDifferentString);
auto simpleTest80 = simple->add("TestRunnerExecutesTests", TestRunnerExecutesTests);
auto simpleTest81 = simple->add("TestContainsWhenSubstringExists", TestContainsWhenSubstringExists);
auto simpleTest82 = simple->add("TestContainsWhenSubstringDoesNotExist", TestContainsWhenSubstringDoesNotExist);


using namespace Parameterized;
auto parameter = CaraTest::getSuite("ParameterTests");
auto parameterTest1 = parameter->add("TestCanExecuteWithParameters", TestCanExecuteWithParameters, TestCanExecuteWithParameters_Data);
auto parameterTest2 = parameter->add("TestWithTwoEmptySubTestsHasTwoPasses", TestWithTwoEmptySubTestsHasTwoPasses);
auto parameterTest3 = parameter->add("TestWithSkipOnTrueInThreeSubTestsHasTwoPassesAndOneSkip", TestWithSkipOnTrueInThreeSubTestsHasTwoPassesAndOneSkip);
auto parameterTest4 = parameter->add("TestWithFailInTwoSubTestsHasTwoFails", TestWithFailInTwoSubTestsHasTwoFails);
auto parameterTest5 = parameter->add("TestWithIsTrueCheckInTwoSubTestsHasOnePassAndOneFails", TestWithIsTrueCheckInTwoSubTestsHasOnePassAndOneFail);
auto parameterTest6 = parameter->add("TestWithSkipOnTrueAndFailHasOneSkipAndTwoFails", TestWithSkipOnTrueAndFailHasOneSkipAndTwoFails);
auto parameterTest7 = parameter->add("TestWithSkipInTwoSubTestsHasTwoSkips", TestWithSkipInTwoSubTestsHasTwoSkips);
auto parameterTest8 = parameter->add("TestWithoutDataDontExecute", TestWithoutDataDontExecute);



auto snapshot = CaraTest::getSuite("SnapshotTests");
auto snapshotTest1 = snapshot->add("EqualsFileCreatesSnapshotWhenFileDoesNotExist", EqualsFileCreatesSnapshotWhenFileDoesNotExist);
auto snapshotTest2 = snapshot->add("EqualsFileDoesntCreateSnapshotWhenValuesareEqual", EqualsFileDoesntCreateSnapshotWhenValuesareEqual);
auto snapshotTest3 = snapshot->add("EqualsFileCreateSnapshotWhenValuesAreNotEqual", EqualsFileCreateSnapshotWhenValuesAreNotEqual);


// CaraTest::getSuite
auto globalSuite1 = CaraTest::getSuite();
auto globalSuite2 = CaraTest::getSuite("__GlobalSuiteName");

auto helper = CaraTest::getSuite("HelperTests");
auto helper1 = helper->add("GlobalGetSuiteWithoutNameReturnsFileName", []()
    {
        CaraTest::areEqual("CaraTestTests", globalSuite1->name());
    });
auto helper2 = helper->add("GlobalGetSuiteWithNameReturnsGivenName", []()
    {
        CaraTest::areEqual("__GlobalSuiteName", globalSuite2->name());
    });

static void LocalGetSuiteWithoutNameReturnsFunctionName()
{
    auto localSuite = CaraTest::getSuite();

    auto hasFunctionName = localSuite->name().find("LocalGetSuiteWithoutNameReturnsFunctionName") != std::string::npos;
    CaraTest::isTrue(hasFunctionName);
    
    // We cant test on the exact name because each compiler returns a different name
    //CaraTest::areEqual("LocalGetSuiteWithoutNameReturnsFunctionName", localSuite->name());
}
auto helper3 = helper->add("LocalGetSuiteWithoutNameReturnsFunctionName", LocalGetSuiteWithoutNameReturnsFunctionName);
auto helper4 = helper->add("LocalGetSuiteWithNameReturnsGivenName", []()
    {
        auto localSuite = CaraTest::getSuite("LocalSuiteName");
        CaraTest::areEqual("LocalSuiteName", localSuite->name());
    });


// CaraTest::addTest
// Version 1: addTest("testName", testFunction)
auto globalTest1 = CaraTest::addTest("__GlobalNamedTest_const_char*", []() {CaraTest::skip(); });
auto helper5 = helper->add("GlobalAddTestWithName_const_char*", []()
    {
        CaraTest::areEqual("__GlobalNamedTest_const_char*", globalTest1->name());
    });

auto globalTest2 = CaraTest::addTest(std::string("__GlobalNamedTest_std::string"), []() {CaraTest::skip(); });
auto helper6 = helper->add("GlobalAddTestWithName_std::string", []()
    {
        CaraTest::areEqual("__GlobalNamedTest_std::string", globalTest2->name());
    });

// Version 2: addTest(testFunction) - uses file and line as test name
auto globalTest3 = CaraTest::addTest([]() {CaraTest::skip(); });
auto helper7 = helper->add("GlobalAddTestWithoutName", []()
    {
        CaraTest::areEqual("CaraTestTests.cpp - Line: 898", globalTest3->name());
    });

// Version 3: addTest(testName, testFunction, data)
static std::vector<std::tuple<bool>> Dummy_Data()
{
    return { std::make_tuple(true), std::make_tuple(false) };
}

auto globalTest4 = CaraTest::addTest("__GlobalNamedParameterTest", [](bool) {CaraTest::skip(); }, Dummy_Data);
auto helper8 = helper->add("GlobalAddTestWithNameAndData_const_char*", []()
    {
        CaraTest::areEqual("__GlobalNamedParameterTest", globalTest4->name());
    });

auto globalTest5 = CaraTest::addTest(std::string("__GlobalNamedParameterTest"), [](bool) {CaraTest::skip(); }, Dummy_Data);
auto helper9 = helper->add("GlobalAddTestWithNameAndData_std::string", []()
    {
        CaraTest::areEqual("__GlobalNamedParameterTest", globalTest5->name());
    });

// Version 4: addTest(testFunction, data)
auto globalTest6 = CaraTest::addTest([](bool) {CaraTest::skip(); }, Dummy_Data);
auto helper10 = helper->add("GlobalAddTestWithNameAndData_const_char*", []()
    {
        CaraTest::areEqual("CaraTestTests.cpp - Line: 923", globalTest6->name());
    });
