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
import dev.olsontek.econbadge.data.LedBorderAnimation
import dev.olsontek.econbadge.databinding.FragmentLedborderAnimationsBinding

class AnimationsFragment : Fragment() {

    private var _binding: FragmentLedborderAnimationsBinding? = null

    private val binding get() = _binding!!

    /* Fragment view model that contains the data to display */
    private lateinit var ledBorderViewModel: LedBorderViewModel

    /* eConBadge manager */
    private lateinit var ecbManager: ECBManager

    /* List of animation models for the recyclerview */
    private val animationList = ArrayList<AnimationListModel>()

    /* List adapter */
    private lateinit var listAdapter: AnimationListAdapter

    /* UI Components */
    private lateinit var animationListView: RecyclerView

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {

        _binding = FragmentLedborderAnimationsBinding.inflate(inflater, container, false)
        val root: View = binding.root

        /* Get the manager and view model */
        ecbManager = ECBManager.getInstance(null)
        ledBorderViewModel = ecbManager.getLedBorderViewModel()

        /* Get the UI components */
        animationListView = binding.animationListView

        /* Setup the list adapter */
        listAdapter = AnimationListAdapter(
            requireActivity(),
            animationList,
            requireActivity().supportFragmentManager,
            ecbManager
        )
        animationListView.adapter = listAdapter
        animationListView.setLayoutManager(LinearLayoutManager(context));
        animationListView.addItemDecoration(
            MaterialDividerItemDecoration(
                requireContext(),
                DividerItemDecoration.VERTICAL
            ).apply {
                dividerInsetEnd = 0
                dividerInsetStart = 0
            }
        )

        /* Setup the animations list observer */
        ledBorderViewModel.animationValues.observe(viewLifecycleOwner) {
            animationList.clear()

            /* Add the "New animation" entry */
            animationList.add(AnimationListModel(-1, "", ""))

            /* Add all animations */
            for (anim in it) {
                val animationData = anim.value
                val name = animationData.name.ifEmpty {
                    animationData.uniqueId.toString()
                }

                var description = if (animationData.type == LedBorderAnimation.AnimationType.TRAIL) {
                    "Trail"
                } else {
                    "Breath"
                }

                description += " | " +
                        animationData.startLedIdx.toString() +
                        " -> " +
                        animationData.endLedIdx.toString() +
                        " | Speed: " +
                        animationData.speed.toString()

                animationList.add(
                    AnimationListModel(
                        animationData.uniqueId,
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