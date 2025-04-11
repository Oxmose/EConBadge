package dev.olsontek.econbadge.ui.ledborder

import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentActivity
import androidx.viewpager2.adapter.FragmentStateAdapter

class LedBorderPagerAdapter(fragmentActivity: FragmentActivity) : FragmentStateAdapter(
    fragmentActivity
) {
    override fun createFragment(position: Int): Fragment {
        // Determine which fragment to show based on position
        return when (position) {
            0 -> AnimationsFragment() // First page
            1 -> PatternsFragment() // Second page
            else -> ScenesFragment() // Third page
        }
    }

    override fun getItemCount(): Int {
        return 3 // Number of fragments (pages)
    }

    fun getPageTitle(position: Int): CharSequence {
        return when (position) {
            0 -> "Animations"
            1 -> "Patterns"
            else -> "Scenes"
        }
    }
}