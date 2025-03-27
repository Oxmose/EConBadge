package dev.olsontek.econbadge.ui.ledborder

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.DividerItemDecoration
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.divider.MaterialDividerItemDecoration
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.data.LedBorderPattern
import dev.olsontek.econbadge.databinding.FragmentLedborderPatternsBinding

class PatternsFragment : Fragment() {

    private var _binding: FragmentLedborderPatternsBinding? = null

    private val binding get() = _binding!!

    /* Fragment view model that contains the data to display */
    private lateinit var ledBorderViewModel: LedBorderViewModel

    /* eConBadge manager */
    private lateinit var ecbManager: ECBManager

    /* List of patterns models for the recyclerview */
    private val patternList = ArrayList<PatternListModel>()

    /* List adapter */
    private lateinit var listAdapter: PatternListAdapter

    /* UI Components */
    private lateinit var patternListView: RecyclerView

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {

        _binding = FragmentLedborderPatternsBinding.inflate(inflater, container, false)
        val root: View = binding.root

        /* Get the manager and view model */
        ecbManager = ECBManager.getInstance(null)
        ledBorderViewModel = ecbManager.getLedBorderViewModel()

        /* Get the UI components */
        patternListView = binding.patternListView

        /* Setup the list adapter */
        listAdapter = PatternListAdapter(
            requireActivity(),
            patternList,
            requireActivity().supportFragmentManager,
            ecbManager
        )
        patternListView.adapter = listAdapter
        patternListView.setLayoutManager(LinearLayoutManager(context));
        patternListView.addItemDecoration(
            MaterialDividerItemDecoration(
                requireContext(),
                DividerItemDecoration.VERTICAL
            ).apply {
                dividerInsetEnd = 0
                dividerInsetStart = 0
            }
        )

        /* Setup the patterns list observer */
        ledBorderViewModel.patternValues.observe(viewLifecycleOwner) {
            patternList.clear()

            /* Add the "New pattern" entry */
            patternList.add(PatternListModel(-1, "", ""))

            /* Add all patterns */
            for (pattern in it) {
                val patternData = pattern.value
                val name = patternData.name.ifEmpty {
                    patternData.uniqueId.toString()
                }

                var description = if (patternData.type == LedBorderPattern.PatternType.PLAIN) {
                    "Plain"
                } else {
                    "Gradient"
                }

                description += " | " +
                        patternData.startLedIdx.toString() +
                        " -> " +
                        patternData.endLedIdx.toString() +
                        " |"

                patternList.add(
                    PatternListModel(
                        patternData.uniqueId,
                        name,
                        description
                    )
                )
            }
            listAdapter.notifyDataSetChanged()
        }

        return root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}