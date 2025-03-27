package dev.olsontek.econbadge.ui.ledborder

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.DividerItemDecoration
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.button.MaterialButton
import com.google.android.material.divider.MaterialDividerItemDecoration
import com.google.android.material.textview.MaterialTextView
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.data.LedBorderScene
import dev.olsontek.econbadge.databinding.FragmentLedborderScenesBinding

class ScenesFragment : Fragment() {

    private var _binding: FragmentLedborderScenesBinding? = null

    private val binding get() = _binding!!

    /* Fragment view model that contains the data to display */
    private lateinit var ledBorderViewModel: LedBorderViewModel

    /* eConBadge manager */
    private lateinit var ecbManager: ECBManager

    /* List of scene models for the recyclerview */
    private val sceneList = ArrayList<SceneListModel>()

    /* List adapter */
    private lateinit var listAdapter: SceneListAdapter

    /* UI Components */
    private lateinit var sceneListView: RecyclerView
    private lateinit var currentSceneTitle: MaterialTextView
    private lateinit var currentSceneDescription: MaterialTextView
    private lateinit var currentSceneEditButton: MaterialButton
    private lateinit var currentSceneDeleteButton: MaterialButton


    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {

        _binding = FragmentLedborderScenesBinding.inflate(inflater, container, false)
        val root: View = binding.root

        /* Get the manager and view model */
        ecbManager = ECBManager.getInstance(null)
        ledBorderViewModel = ecbManager.getLedBorderViewModel()

        /* Get the UI components */
        sceneListView = binding.sceneListView
        currentSceneTitle = binding.currentSceneTitle
        currentSceneDescription = binding.currentSceneDescription
        currentSceneEditButton = binding.currentSceneEdit
        currentSceneDeleteButton = binding.currentSceneDelete

        /* Setup the list adapter */
        listAdapter = SceneListAdapter(
            requireActivity(),
            sceneList,
            requireActivity().supportFragmentManager,
            ecbManager
        )
        sceneListView.adapter = listAdapter
        sceneListView.setLayoutManager(LinearLayoutManager(context));
        sceneListView.addItemDecoration(
            MaterialDividerItemDecoration(
                requireContext(),
                DividerItemDecoration.VERTICAL
            ).apply {
                dividerInsetEnd = 0
                dividerInsetStart = 0
            }
        )

        /* Setup the scene list observer */
        ledBorderViewModel.sceneValues.observe(viewLifecycleOwner) {
            sceneList.clear()

            /* Add the "New Scene" entry */
            sceneList.add(SceneListModel(-1, "", ""))

            /* Add all scenes */
            for (scene in it) {
                val sceneData = scene.value
                val name = sceneData.name.ifEmpty {
                    sceneData.uniqueId.toString()
                }

                val description = "Patterns: " + sceneData.patterns.size.toString() +
                        " | Animations: " + sceneData.animations.size.toString()


                sceneList.add(
                    SceneListModel(
                        sceneData.uniqueId,
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