# Augmented XML reporter

The Augmented XML reporter is a test reporter for Catch2 based on its built-in XML reporter, but with a greater focus on test runs.

The reporter is primarily intended for use with [Umbra's](https://codeberg.org/LunarWatcher/umbra) test module.

## Format rationale

Catch2's XML reporter is likely the single most capable reporter. This makes it easier to refine the existing format to make it consumable by test tools.

The augmented reporter changes the format to be focused on actual runs, rather than sections. This is because Catch2's general test format makes it very hard to differentiate a test run from a section run.

Consider
```cpp
TEST_CASE("Test") {
    SECTION("Whatever") {
        int x;
        SECTION("x = 1") { x = 1; }
        SECTION("x = 2") { x = 2; }
        SECTION("x = 3") { x = 3; }

        REQUIRE(x != 2);
    }
}
```

Currently, this will not be considered a test failure of the `x = 2` section, but of the `Whatever` section. I really like the sections in Catch2, but they do not lend themselves to making the source of a test failure clear. A section-oriented reporter, like what we see in the built-in XML reporter, cannot correctly report the source of the `x != 2` unless being run-focused. Though the tests could be interpreted as a tree (and I _really_ want to do precisely that), the fact the actual tests can be incorrectly reported as passed complicates the tree structure.

The built-in test reporter also partly misreports `StdOut` and `StdErr` due to this, and attributes the output to the test case rather than to the individual runs. This is fine with uncomplicated section structures, but in complicated section structures, and especially when something fails, the erasure is a problem. At the cost of the nice tree structure, a run-focused test reporter makes it easier to correctly attribute output and similar to the correct test that failed rather than something else.

Also, in practice, the XML reporter already had parts of this, but with arbitrary nesting that's harder to parse by tools. Each nested list of `<Section>` indicates the path for a test run. This information is still present, just in a flat list under a `<TestRun>` element.

## Example output

This is based on real output from a limited run on stc's test suite, with comments added to highlight differences

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!--
The root tag is still identical, with the exception of the added `generator` field.
Unfortunately, the schema is not compatible with the proper XML reporter, though the core
elements are still kept the same because I'm not sure how to best differentiate the built-in
XML reporter from the augmented XML reporter.

For now, the extra `generator` field can be used to exclude the augmented XML from tools that
require the builtin XML (though I doubt it'll ever come up).
-->
<Catch2TestRun name="tests" rng-seed="3454557717" xml-format-version="3" catch2-version="3.14.0" generator="stc::testutil::AugmentedXMLReporter" filters="[Math]">
  <TestCase name="Point in 4-point rectangle" tags="[2D geometry][Math]" filename="/home/olivia/programming/cpp/stc/tests/src/math/2DGeometryTests.cpp" line="123">
    <!-- TestCase gets an extra TestRun when sections are run. -->
    <TestRun run-number="0">
      <!-- Sections have been flattened into a single line. A TestRun can therefore have
           multiple Section elements. These will always be ordered by depth, so the first
           Section element will always be the outermost Section, and the last Section
           element will always be the innermost section.

           These are not nested so each test run can have a flat structure, since each test
           run contains the interesting data, rather than each section
      -->
      <Section name="Exclusive" filename="/home/olivia/programming/cpp/stc/tests/src/math/2DGeometryTests.cpp" line="124"/>

      <!-- Duration is a new field that describes how long the test took to run -->
      <Duration durationInSeconds="6.9e-05"/>
      <!-- OverallResults gets an extra success field. -->
      <OverallResults successes="1" failures="0" expectedFailures="0" skipped="false" success="true"/>
    </TestRun>
    <TestRun run-number="1">
      <Section name="Inclusive" filename="/home/olivia/programming/cpp/stc/tests/src/math/2DGeometryTests.cpp" line="136"/>
      <Duration durationInSeconds="1.4e-05"/>
      <OverallResults successes="1" failures="0" expectedFailures="0" skipped="false" success="true"/>
    </TestRun>
    <OverallResult success="true" skips="0"/>
  </TestCase>
  <TestCase name="Point in 2-point rectangle" tags="[2D geometry][Math]" filename="/home/olivia/programming/cpp/stc/tests/src/math/2DGeometryTests.cpp" line="151">
    <TestRun run-number="0">
      <!-- The implicit root section is not listed, so TestRuns can contain no sections. -->
      <Duration durationInSeconds="2.3e-05"/>
      <OverallResults successes="19" failures="0" expectedFailures="0" skipped="false" success="true"/>
    </TestRun>
    <OverallResult success="true" skips="0"/>
  </TestCase>
  <TestCase name="Line-Rectangle intersection" tags="[2D geometry][Math]" filename="/home/olivia/programming/cpp/stc/tests/src/math/2DGeometryTests.cpp" line="76">
    <TestRun run-number="0">
      <Duration durationInSeconds="7e-06"/>
      <OverallResults successes="2" failures="0" expectedFailures="0" skipped="false" success="true"/>
    </TestRun>
    <OverallResult success="true" skips="0"/>
  </TestCase>
  <TestCase name="Line-line intersection" tags="[2D geometry][Math]" filename="/home/olivia/programming/cpp/stc/tests/src/math/2DGeometryTests.cpp" line="7">
    <TestRun run-number="0">
      <Duration durationInSeconds="9e-06"/>
      <OverallResults successes="8" failures="0" expectedFailures="0" skipped="false" success="true"/>
    </TestRun>
    <OverallResult success="true" skips="0"/>
  </TestCase>
  <TestCase name="Sidedness" tags="[2D geometry][Math]" filename="/home/olivia/programming/cpp/stc/tests/src/math/2DGeometryTests.cpp" line="101">
    <TestRun run-number="0">
      <Duration durationInSeconds="3.3e-05"/>
      <OverallResults successes="2" failures="0" expectedFailures="0" skipped="false" success="true"/>
    </TestRun>
    <OverallResult success="true" skips="0"/>
  </TestCase>
  <OverallResults successes="33" failures="0" expectedFailures="0" skips="0"/>
  <OverallResultsCases successes="5" failures="0" expectedFailures="0" skips="0"/>
</Catch2TestRun>
```

### Fixture example
Note that only the TestCase is shown here, and not the surrounding context, to avoid repeating the existing example.

```xml
<!-- Note the added class-name attribute -->
<TestCase name="Test" tags="[Math]" class-name="Fixture" filename="/home/olivia/programming/cpp/stc/tests/src/math/2DGeometryTests.cpp" line="9">
  <TestRun run-number="0">
    <Section name="hi" filename="/home/olivia/programming/cpp/stc/tests/src/math/2DGeometryTests.cpp" line="10"/>
    <Duration durationInSeconds="2.5e-05"/>
    <OverallResults successes="1" failures="0" expectedFailures="0" skipped="false" success="true"/>
  </TestRun>
  <OverallResult success="true" skips="0"/>
</TestCase>
```

Whether or not you make use of this attribute is entirely up to you. If anything, treat it as an extra hierarchy node. Please note, however, that the class-name may not be unique. There's no way to identify the source of the fixture, so if you define multiple fixtures with the same name in your test suite, it will not link properly.
