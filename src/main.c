/*
 * the MIT License (MIT)
 *
 * Copyright (c) 2023 Trent Summerlin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * the above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "runtime/runtime.h"

int
main(int argc, const char *argv[])
{
    rbtk_init_runtime();
    int result = rbtk_runtime_main(argc, argv);
    rbtk_terminate_runtime();
    return result;
}

#if defined(_WIN32)

#include <windows.h>

int WINAPI
WinMain(RBTK_UNUSED _In_ HINSTANCE hInstance,
    RBTK_UNUSED _In_opt_ HINSTANCE hPrevInstance,
    RBTK_UNUSED _In_ PSTR lpCmdLine,
    RBTK_UNUSED _In_ int nCmdShow)
{
    return main(__argc, __argv);
}

#endif /* defined(_WIN32) */
