package dev.olsontek.econbadge.ui.ledborder

import android.app.AlertDialog
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.viewpager2.widget.ViewPager2
import com.google.android.material.tabs.TabLayout
import com.google.android.material.tabs.TabLayoutMediator
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.databinding.FragmentLedborderBinding


class LebBorderMainFragment : Fragment() {

    private var _binding: FragmentLedborderBinding? = null

    private val binding get() = _binding!!

    /* eConBadge manager */
    private lateinit var ecbManager: ECBManager

    private lateinit var viewPager : ViewPager2
    private lateinit var viewPagerAdapter: LedBorderPagerAdapter
    private lateinit var viewPagerTabs: TabLayout

    /* Information retrieval dialog */
    private lateinit var infoRetrieveDialog: AlertDialog

    private fun displayInfoDialog(title: String, content: String) {
        val builder = AlertDialog.Builder(requireActivity())
        builder.setTitle(title)
        builder.setMessage(content)
        builder.setCancelable(false)

        infoRetrieveDialog = builder.create()
        infoRetrieveDialog.show()
    }

    private fun hideInfoDialog() {
        infoRetrieveDialog.dismiss()
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {

        _binding = FragmentLedborderBinding.inflate(inflater, container, false)
        val root: View = binding.root

        viewPager = binding.ledBorderViewPager
        viewPagerAdapter = LedBorderPagerAdapter(requireActivity())
        viewPagerTabs = binding.ledBorderTabLayout

        viewPager.adapter = viewPagerAdapter

        TabLayoutMediator(viewPagerTabs, viewPager) { tab, position ->
            tab.text = viewPagerAdapter.getPageTitle(position)
            viewPager.setCurrentItem(tab.position, true)
        }.attach()

        /* Get the data */
        ecbManager = ECBManager.getInstance(null)

        displayInfoDialog(
            getString(R.string.retrieving_data),
            getString(R.string.retrieving_data_please_wait)
        )
        ecbManager.retrieveLedBorderFragmentData {
            hideInfoDialog()
        }

        return root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}