//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Framebuffer multiview tests:
// The tests modify and examine the multiview state.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class FramebufferMultiviewTest : public ANGLETest
{
  protected:
    FramebufferMultiviewTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setWebGLCompatibilityEnabled(true);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();
        glRequestExtensionANGLE = reinterpret_cast<PFNGLREQUESTEXTENSIONANGLEPROC>(
            eglGetProcAddress("glRequestExtensionANGLE"));
    }

    // Requests the ANGLE_multiview extension and returns true if the operation succeeds.
    bool requestMultiviewExtension()
    {
        if (extensionRequestable("GL_ANGLE_multiview"))
        {
            glRequestExtensionANGLE("GL_ANGLE_multiview");
        }

        if (!extensionEnabled("GL_ANGLE_multiview"))
        {
            std::cout << "Test skipped due to missing GL_ANGLE_multiview." << std::endl;
            return false;
        }
        return true;
    }

    PFNGLREQUESTEXTENSIONANGLEPROC glRequestExtensionANGLE = nullptr;
};

// Test that the framebuffer tokens introduced by ANGLE_multiview can be used query the framebuffer
// state and that their corresponding default values are correctly set.
TEST_P(FramebufferMultiviewTest, DefaultState)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    GLint numViews = -1;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_ANGLE,
                                          &numViews);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(1, numViews);

    GLint baseViewIndex = -1;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_ANGLE,
                                          &baseViewIndex);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(0, baseViewIndex);

    GLint multiviewLayout = GL_FRAMEBUFFER_MULTIVIEW_SIDE_BY_SIDE_ANGLE;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_MULTIVIEW_LAYOUT_ANGLE,
                                          &multiviewLayout);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(GL_NONE, multiviewLayout);

    GLint viewportOffsets[2] = {-1, -1};
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_VIEWPORT_OFFSETS_ANGLE,
                                          &viewportOffsets[0]);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(0, viewportOffsets[0]);
    EXPECT_EQ(0, viewportOffsets[1]);
}

// Test that without having the ANGLE_multiview extension, querying for the framebuffer state using
// the ANGLE_multiview tokens results in an INVALID_ENUM error.
TEST_P(FramebufferMultiviewTest, NegativeFramebufferStateQueries)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    GLint numViews = -1;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_ANGLE,
                                          &numViews);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLint baseViewIndex = -1;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_ANGLE,
                                          &baseViewIndex);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLint multiviewLayout = GL_FRAMEBUFFER_MULTIVIEW_SIDE_BY_SIDE_ANGLE;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_MULTIVIEW_LAYOUT_ANGLE,
                                          &multiviewLayout);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLint viewportOffsets[2] = {-1, -1};
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_VIEWPORT_OFFSETS_ANGLE,
                                          &viewportOffsets[0]);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Test that the correct errors are generated whenever glFramebufferTextureMultiviewSideBySideANGLE
// is called with invalid arguments.
TEST_P(FramebufferMultiviewTest, InvalidMultiviewSideBySideArguments)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Negative offsets.
    GLint viewportOffsets[2] = {-1};
    glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 1,
                                                 &viewportOffsets[0]);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Negative number of views.
    viewportOffsets[0] = 0;
    viewportOffsets[1] = 0;
    glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, -1,
                                                 &viewportOffsets[0]);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Test that the correct errors are generated whenever glFramebufferTextureMultiviewLayeredANGLE is
// called with invalid arguments.
TEST_P(FramebufferMultiviewTest, InvalidMultiviewLayeredArguments)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 1, 1, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    // Negative base view index.
    glFramebufferTextureMultiviewLayeredANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, -1, 1);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // baseViewIndex + numViews is greater than MAX_TEXTURE_LAYERS.
    GLint maxTextureLayers = 0;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTextureLayers);
    glFramebufferTextureMultiviewLayeredANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0,
                                              maxTextureLayers, 1);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Test that an INVALID_OPERATION error is generated whenever the ANGLE_multiview extension is not
// available.
TEST_P(FramebufferMultiviewTest, ExtensionNotAvailableCheck)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    ASSERT_GL_NO_ERROR();
    const GLint kViewportOffsets[2] = {0};
    glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 1,
                                                 &kViewportOffsets[0]);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that glFramebufferTextureMultiviewSideBySideANGLE modifies the internal multiview state.
TEST_P(FramebufferMultiviewTest, ModifySideBySideState)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    const GLint kViewportOffsets[4] = {0, 0, 1, 2};
    glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 2,
                                                 &kViewportOffsets[0]);
    ASSERT_GL_NO_ERROR();

    GLint numViews = -1;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_ANGLE,
                                          &numViews);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(2, numViews);

    GLint baseViewIndex = -1;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_ANGLE,
                                          &baseViewIndex);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(0, baseViewIndex);

    GLint multiviewLayout = GL_NONE;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_MULTIVIEW_LAYOUT_ANGLE,
                                          &multiviewLayout);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(GL_FRAMEBUFFER_MULTIVIEW_SIDE_BY_SIDE_ANGLE, multiviewLayout);

    GLint internalViewportOffsets[4] = {-1};
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_VIEWPORT_OFFSETS_ANGLE,
                                          &internalViewportOffsets[0]);
    ASSERT_GL_NO_ERROR();
    for (size_t i = 0u; i < 4u; ++i)
    {
        EXPECT_EQ(kViewportOffsets[i], internalViewportOffsets[i]);
    }
}

// Test framebuffer completeness status of a side-by-side framebuffer with color and depth
// attachments.
TEST_P(FramebufferMultiviewTest, IncompleteViewTargetsSideBySide)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    const GLint kViewportOffsets[4]      = {0, 0, 2, 0};
    const GLint kOtherViewportOffsets[4] = {2, 0, 4, 0};

    // Set the 0th attachment and keep it as it is till the end of the test. The 1st or depth
    // attachment will me modified to change the framebuffer's status.
    glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 2,
                                                 &kViewportOffsets[0]);
    ASSERT_GL_NO_ERROR();

    // Color attachment 1.
    {
        GLTexture otherTex;
        glBindTexture(GL_TEXTURE_2D, otherTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // Test framebuffer completeness when the number of views differ.
        glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, otherTex,
                                                     0, 1, &kViewportOffsets[0]);
        ASSERT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_ANGLE,
                         glCheckFramebufferStatus(GL_FRAMEBUFFER));

        // Test framebuffer completeness when the viewport offsets differ.
        glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, otherTex,
                                                     0, 2, &kOtherViewportOffsets[0]);
        ASSERT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_ANGLE,
                         glCheckFramebufferStatus(GL_FRAMEBUFFER));

        // Test framebuffer completeness when attachment layouts differ.
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, otherTex, 0);
        ASSERT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_ANGLE,
                         glCheckFramebufferStatus(GL_FRAMEBUFFER));

        // Test that framebuffer is complete when the number of views, viewport offsets and layouts
        // are the same.
        glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, otherTex,
                                                     0, 2, &kViewportOffsets[0]);
        ASSERT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

        // Reset attachment 1
        glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0, 1,
                                                     &kViewportOffsets[0]);
    }

    // Depth attachment.
    {
        GLTexture depthTex;
        glBindTexture(GL_TEXTURE_2D, depthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                     nullptr);

        // Test framebuffer completeness when the number of views differ.
        glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex,
                                                     0, 1, &kViewportOffsets[0]);
        ASSERT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_ANGLE,
                         glCheckFramebufferStatus(GL_FRAMEBUFFER));

        // Test framebuffer completeness when the viewport offsets differ.
        glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex,
                                                     0, 2, &kOtherViewportOffsets[0]);
        ASSERT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_ANGLE,
                         glCheckFramebufferStatus(GL_FRAMEBUFFER));

        // Test framebuffer completeness when attachment layouts differ.
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
        ASSERT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_ANGLE,
                         glCheckFramebufferStatus(GL_FRAMEBUFFER));

        // Test that framebuffer is complete when the number of views, viewport offsets and layouts
        // are the same.
        glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex,
                                                     0, 2, &kViewportOffsets[0]);
        ASSERT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
}

// Test that the active read framebuffer cannot be read from through glCopyTex* if it has multi-view
// attachments.
TEST_P(FramebufferMultiviewTest, InvalidCopyTex)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    const GLint kViewportOffsets[2] = {0};
    glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 1,
                                                 &kViewportOffsets[0]);
    ASSERT_GL_NO_ERROR();

    // Test glCopyTexImage2D and glCopyTexSubImage2D.
    {
        GLTexture tex2;
        glBindTexture(GL_TEXTURE_2D, tex2);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, 1, 1, 0);
        EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);

        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1, 1);
        EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
    }

    // Test glCopyTexSubImage3D.
    {
        GLTexture tex2;
        glBindTexture(GL_TEXTURE_3D, tex2);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glCopyTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, 0, 0, 1, 1);
        EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
    }
}

// Test that glBlitFramebuffer generates an invalid framebuffer operation when either the current
// draw framebuffer, or current read framebuffer have multiview attachments.
TEST_P(FramebufferMultiviewTest, InvalidBlit)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    const GLint kViewportOffsets[2] = {0};
    glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 1,
                                                 &kViewportOffsets[0]);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    ASSERT_GL_NO_ERROR();

    // Blit with the active read framebuffer having multiview attachments.
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
    }

    // Blit with the active draw framebuffer having multiview attachments.
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
    }
}

// Test that glReadPixels generates an invalid framebuffer operation error if the current read
// framebuffer has a multi-view layout.
TEST_P(FramebufferMultiviewTest, InvalidReadPixels)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    const GLint kViewportOffsets[2] = {0};
    glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 1,
                                                 &kViewportOffsets[0]);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    ASSERT_GL_NO_ERROR();

    GLColor pixelColor;
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixelColor.R);
    EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
}

// Test that glClear clears only the contents of each view.
TEST_P(FramebufferMultiviewTest, SideBySideClear)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    GLFramebuffer multiviewFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, multiviewFBO);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 4, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    const GLint kViewportOffsets[4] = {1, 0, 3, 0};
    glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 2,
                                                 &kViewportOffsets[0]);

    // Create and bind a normal framebuffer to access the 2D texture.
    GLFramebuffer normalFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, normalFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    // Clear the contents of the texture.
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Bind and specify viewport/scissor dimensions for each view.
    glBindFramebuffer(GL_FRAMEBUFFER, multiviewFBO);
    glViewport(0, 0, 1, 2);
    glScissor(0, 0, 1, 2);

    glClearColor(1, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, normalFBO);

    // column 0
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 0, 0);
    EXPECT_PIXEL_EQ(0, 1, 0, 0, 0, 0);

    // column 1
    EXPECT_PIXEL_EQ(1, 0, 255, 0, 0, 0);
    EXPECT_PIXEL_EQ(1, 1, 255, 0, 0, 0);

    // column 2
    EXPECT_PIXEL_EQ(2, 0, 0, 0, 0, 0);
    EXPECT_PIXEL_EQ(2, 1, 0, 0, 0, 0);

    // column 3
    EXPECT_PIXEL_EQ(3, 0, 255, 0, 0, 0);
    EXPECT_PIXEL_EQ(3, 1, 255, 0, 0, 0);
}

ANGLE_INSTANTIATE_TEST(FramebufferMultiviewTest, ES3_OPENGL());