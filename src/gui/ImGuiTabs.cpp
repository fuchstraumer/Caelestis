#include "gui/ImGuiTabs.hpp"

namespace ImGui {

    IMGUI_API bool TabLabels(const char ** tabLabels, int tabSize, int & tabIndex, int * tabOrder)
    {
        ImGuiStyle& style = ImGui::GetStyle();

        const ImVec2 itemSpacing = style.ItemSpacing;
        const ImVec4 color = style.Colors[ImGuiCol_Button];
        const ImVec4 colorActive = style.Colors[ImGuiCol_ButtonActive];
        const ImVec4 colorHover = style.Colors[ImGuiCol_ButtonHovered];
        const ImVec4 colorText = style.Colors[ImGuiCol_Text];
        style.ItemSpacing.x = 1;
        style.ItemSpacing.y = 1;
        const ImVec4 colorSelectedTab = ImVec4(color.x, color.y, color.z, color.w*0.5f);
        const ImVec4 colorSelectedTabHovered = ImVec4(colorHover.x, colorHover.y, colorHover.z, colorHover.w*0.5f);
        const ImVec4 colorSelectedTabText = ImVec4(colorText.x*0.8f, colorText.y*0.8f, colorText.z*0.8f, colorText.w*0.8f);

        if (tabSize>0 && (tabIndex<0 || tabIndex >= tabSize)) {
            if (!tabOrder)  tabIndex = 0;
            else tabIndex = -1;
        }

        float windowWidth = 0.f, sumX = 0.f;
        windowWidth = ImGui::GetWindowWidth() - style.WindowPadding.x - (ImGui::GetScrollMaxY()>0 ? style.ScrollbarSize : 0.f);

        static int draggingTabIndex = -1; int draggingTabTargetIndex = -1;   // These are indices inside tabOrder
        static ImVec2 draggingtabSize(0, 0);
        static ImVec2 draggingTabOffset(0, 0);

        const bool isMMBreleased = ImGui::IsMouseReleased(2);
        const bool isMouseDragging = ImGui::IsMouseDragging(0, 2.f);
        int justClosedTabIndex = -1, newtabIndex = tabIndex;


        bool selection_changed = false; bool noButtonDrawn = true;
        for (int j = 0, i; j < tabSize; j++)
        {
            i = tabOrder ? tabOrder[j] : j;
            if (i == -1) continue;

            if (sumX > 0.f) {
                sumX += style.ItemSpacing.x;   // Maybe we can skip it if we use SameLine(0,0) below
                sumX += ImGui::CalcTextSize(tabLabels[i]).x + 2.f*style.FramePadding.x;
                if (sumX>windowWidth) sumX = 0.f;
                else ImGui::SameLine();
            }

            if (i != tabIndex) {
                // Push the style
                style.Colors[ImGuiCol_Button] = colorSelectedTab;
                style.Colors[ImGuiCol_ButtonActive] = colorSelectedTab;
                style.Colors[ImGuiCol_ButtonHovered] = colorSelectedTabHovered;
                style.Colors[ImGuiCol_Text] = colorSelectedTabText;
            }
            // Draw the button
            ImGui::PushID(i);   // otherwise two tabs with the same name would clash.
            if (ImGui::Button(tabLabels[i])) { selection_changed = (tabIndex != i); newtabIndex = i; }
            ImGui::PopID();
            if (i != tabIndex) {
                // Reset the style
                style.Colors[ImGuiCol_Button] = color;
                style.Colors[ImGuiCol_ButtonActive] = colorActive;
                style.Colors[ImGuiCol_ButtonHovered] = colorHover;
                style.Colors[ImGuiCol_Text] = colorText;
            }
            noButtonDrawn = false;

            if (sumX == 0.f) sumX = style.WindowPadding.x + ImGui::GetItemRectSize().x; // First element of a line

            if (ImGui::IsItemHoveredRect()) {
                if (tabOrder) {
                    // tab reordering
                    if (isMouseDragging) {
                        if (draggingTabIndex == -1) {
                            draggingTabIndex = j;
                            draggingtabSize = ImGui::GetItemRectSize();
                            const ImVec2& mp = ImGui::GetIO().MousePos;
                            const ImVec2 draggingTabCursorPos = ImGui::GetCursorPos();
                            draggingTabOffset = ImVec2(
                                mp.x + draggingtabSize.x*0.5f - sumX + ImGui::GetScrollX(),
                                mp.y + draggingtabSize.y*0.5f - draggingTabCursorPos.y + ImGui::GetScrollY()
                            );

                        }
                    }
                    else if (draggingTabIndex >= 0 && draggingTabIndex<tabSize && draggingTabIndex != j) {
                        draggingTabTargetIndex = j; // For some odd reasons this seems to get called only when draggingTabIndex < i ! (Probably during mouse dragging ImGui owns the mouse someway and sometimes ImGui::IsItemHovered() is not getting called)
                    }
                }
            }

        }

        tabIndex = newtabIndex;

        // Draw tab label while mouse drags it
        if (draggingTabIndex >= 0 && draggingTabIndex<tabSize) {
            const ImVec2& mp = ImGui::GetIO().MousePos;
            const ImVec2 wp = ImGui::GetWindowPos();
            ImVec2 start(wp.x + mp.x - draggingTabOffset.x - draggingtabSize.x*0.5f, wp.y + mp.y - draggingTabOffset.y - draggingtabSize.y*0.5f);
            const ImVec2 end(start.x + draggingtabSize.x, start.y + draggingtabSize.y);
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const float draggedBtnAlpha = 0.65f;
            const ImVec4& btnColor = style.Colors[ImGuiCol_Button];
            drawList->AddRectFilled(start, end, ImColor(btnColor.x, btnColor.y, btnColor.z, btnColor.w*draggedBtnAlpha), style.FrameRounding);
            start.x += style.FramePadding.x; start.y += style.FramePadding.y;
            const ImVec4& txtColor = style.Colors[ImGuiCol_Text];
            drawList->AddText(start, ImColor(txtColor.x, txtColor.y, txtColor.z, txtColor.w*draggedBtnAlpha), tabLabels[tabOrder[draggingTabIndex]]);

            ImGui::SetMouseCursor(ImGuiMouseCursor_Move);
        }

        // Drop tab label
        if (draggingTabTargetIndex != -1) {
            // swap draggingTabIndex and draggingTabTargetIndex in tabOrder
            const int tmp = tabOrder[draggingTabTargetIndex];
            tabOrder[draggingTabTargetIndex] = tabOrder[draggingTabIndex];
            tabOrder[draggingTabIndex] = tmp;
            //fprintf(stderr,"%d %d\n",draggingTabIndex,draggingTabTargetIndex);
            draggingTabTargetIndex = draggingTabIndex = -1;
        }

        // Reset draggingTabIndex if necessary
        if (!isMouseDragging) draggingTabIndex = -1;

        // Change selected tab when user closes the selected tab
        if (tabIndex == justClosedTabIndex && tabIndex >= 0) {
            tabIndex = -1;
            for (int j = 0, i; j < tabSize; j++) {
                i = tabOrder ? tabOrder[j] : j;
                if (i == -1) continue;
                tabIndex = i;
                break;
            }
        }

        // Restore the style
        style.Colors[ImGuiCol_Button] = color;
        style.Colors[ImGuiCol_ButtonActive] = colorActive;
        style.Colors[ImGuiCol_ButtonHovered] = colorHover;
        style.Colors[ImGuiCol_Text] = colorText;
        style.ItemSpacing = itemSpacing;

        return selection_changed;
    }
}
