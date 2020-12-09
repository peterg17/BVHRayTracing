import subprocess
import time
import csv

def main():
  # Example bash command to run raytracing code
  # ./assignment6 -input scene06_bunny_1k.txt -output 06_bvh.png -size 1000 1000 -bounces 15 -indexer bvh_sah
  
  # OCTREE
  octree_script = "./assignment6 -input scene06_bunny_1k.txt -output 06_octree.png -size 2000 2000 -bounces 30 -indexer octree"
  octree_script = octree_script.split()

  # BVH
  bvh_script = "./assignment6 -input scene06_bunny_1k.txt -output 06_bvh.png -size 2000 2000 -bounces 30 -indexer bvh"
  bvh_script = bvh_script.split()

  # BVH_SAH
  bvh_sah_script = "./assignment6 -input scene06_bunny_1k.txt -output 06_bvh_sah.png -size 2000 2000 -bounces 30 -indexer bvh_sah"
  bvh_sah_script = bvh_sah_script.split()

  octree_pairs = []
  bvh_pairs = []
  bvh_sah_pairs = []
  header_names = ['indexer', 'wall_time']
  print()
  print("----------------------------------------------")
  for i in range(100):
    print("Test Run: %d" % (i))
    # collect OCTREE data
    start_time = time.time()
    out = subprocess.run(octree_script, stdout=subprocess.PIPE)
    octree_render_time = time.time() - start_time
    octree_pairs.append({'indexer': 'OCTREE', 'wall_time': octree_render_time})
    print("octree render time: %.3f" % (octree_render_time))
    # collect BVH data
    start_time = time.time()
    out = subprocess.run(bvh_script, stdout=subprocess.PIPE)
    bvh_render_time = time.time() - start_time
    bvh_pairs.append({'indexer': 'BVH', 'wall_time': bvh_render_time})
    print("bvh render time: %.3f" % (bvh_render_time))
    # collect BVH w/ SAH data
    start_time = time.time()
    out = subprocess.run(bvh_sah_script, stdout=subprocess.PIPE)
    bvh_sah_render_time = time.time() - start_time
    bvh_sah_pairs.append({'indexer': 'BVH_SAH', 'wall_time': bvh_sah_render_time})
    print("bvh w/ sah render time: %.3f" % (bvh_sah_render_time))
    print()
    print("----------------------------------------------")

  
  # create CSV with columns:    , indexer, wall time
  #                       ex: 1,  BVH_SAH ,  2.218 s
  #                           2,  OCTREE  ,  2.515 s
  with open('perf_data.csv', 'w') as csvfile:
    writer = csv.DictWriter(csvfile, fieldnames=header_names)
    writer.writeheader()
    for o in octree_pairs:
      writer.writerow(o)
    for b in bvh_pairs:
      writer.writerow(b)
    for b_s in bvh_sah_pairs:
      writer.writerow(b_s)


if __name__ == '__main__':
  # do some stuff
  main()
