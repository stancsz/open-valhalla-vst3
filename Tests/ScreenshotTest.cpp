#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
// cmake --build build --config Debug --target ScreenshotTest
class ScreenshotTestApp : public juce::JUCEApplication
{
public:
    ScreenshotTestApp() {}
    const juce::String getApplicationName() override { return "ScreenshotTest"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        // Create the plugin
        auto plugin = std::make_unique<VST3OpenValhallaAudioProcessor>();
        
        // Create the editor
        // Note: createEditor returns a pointer that we own (usually) or is managed by the processor?
        // AudioProcessor::createEditor() returns a new object that the caller owns.
        auto* editor = plugin->createEditor();
        
        if (editor == nullptr)
        {
            std::cerr << "Failed to create editor." << std::endl;
            quit();
            return;
        }

        // Set bounds. The editor usually sets its own size in constructor, but we should ensure it has a size.
        // If the editor uses setSize in its constructor, it should be fine.
        // Let's check if it has a size.
        if (editor->getWidth() == 0 || editor->getHeight() == 0)
        {
            editor->setSize(1000, 650); // Default size from typical VSTs, adjust if needed.
        }
        
        // Force a repaint/layout
        editor->setVisible(true);
        
        // We might need to let the message loop run for a moment if there are async updates?
        // But for a simple snapshot, it might be immediate.
        
        auto image = editor->createComponentSnapshot(editor->getLocalBounds());
        
        // Create test directory if it doesn't exist
        juce::File cwd = juce::File::getCurrentWorkingDirectory();
        juce::File testDir = cwd.getChildFile("TestOutput");
        
        if (!testDir.exists())
        {
            auto result = testDir.createDirectory();
            if (result.failed())
            {
                 std::cerr << "Failed to create test directory: " << result.getErrorMessage() << std::endl;
                 delete editor;
                 quit();
                 return;
            }
        }
            
        juce::File screenshotFile = testDir.getChildFile("screenshot.png");
        
        if (screenshotFile.exists())
            screenshotFile.deleteFile();
        
        juce::PNGImageFormat png;
        juce::FileOutputStream stream(screenshotFile);
        
        if (stream.openedOk())
        {
            if (png.writeImageToStream(image, stream))
            {
                std::cout << "Screenshot saved to " << screenshotFile.getFullPathName() << std::endl;
            }
            else
            {
                std::cerr << "Failed to write image to stream." << std::endl;
            }
        }
        else
        {
            std::cerr << "Failed to open stream for " << screenshotFile.getFullPathName() << std::endl;
        }

        delete editor;
        quit();
    }

    void shutdown() override {}
};

START_JUCE_APPLICATION(ScreenshotTestApp)
