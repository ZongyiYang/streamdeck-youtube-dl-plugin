#include "pch.h"

#include "../CurlUtils.hpp"
#include "../RedditDlUtils.h"

namespace Tests
{
    class readHtmlTest :
        public ::testing::TestWithParam<std::pair<std::string, bool>> {};

    INSTANTIATE_TEST_CASE_P(
        TestUrls,
        readHtmlTest,
        ::testing::Values(
            std::make_pair("", false),
            std::make_pair("badurl", false),
            std::make_pair("https://www.google.com", true),
            std::make_pair("https://www.reddit.com/r/LearnToReddit/comments/1fwjffr/pic_test/.json", true),
            std::make_pair("https://www.reddit.com/r/perfectlycutscreams/comments/ilg3z7/thurston_wants_to_go_outside/.json", true)
        )
    );

    TEST_P(readHtmlTest, TestUrls) {
        const auto& [url, result] = GetParam();
        std::unique_ptr<std::string> htmlData;
        htmlData.reset(new std::string());

        EXPECT_EQ(curlutils::readHTML(url, htmlData.get()), result);
    }

    class redditDownloadTest :
        public ::testing::TestWithParam<std::pair<std::string, std::string>> {};

    INSTANTIATE_TEST_CASE_P(
        TestRedditDownload,
        redditDownloadTest,
        ::testing::Values(
            // yt-dlp cannot download reddit images, but this function should be able to with curl
            std::make_pair("https://www.reddit.com/r/LearnToReddit/comments/1fwjffr/pic_test/.json", ""),
            // image downloader should not be able to download videos, we fall back to yt-dlp for that
            std::make_pair("https://www.reddit.com/r/perfectlycutscreams/comments/ilg3z7/thurston_wants_to_go_outside/.json", "Error: reddit webpage does not contain image data.")
        )
    );

    TEST_P(redditDownloadTest, TestDownloading) {
        const auto& [url, resultMsg] = GetParam();

        std::string errMsg;
        struct curlutils::MemoryStruct chunk;
        chunk.memory = (unsigned char*)malloc(1);  /* will be grown as needed by the realloc in the callback */
        chunk.size = 0;    /* no data at this point */
        try
        {
            redditdlutils::downloadRedditContent(url, "./", chunk);
        }
        catch (std::exception& e)
        {
            errMsg = std::string(e.what());
        }

        if (chunk.memory)
            free(chunk.memory);

        EXPECT_EQ(errMsg, resultMsg);
    }
}