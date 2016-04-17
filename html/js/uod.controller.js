(function() {

	angular.module('uod').controller('uodController', [ '$http', '$timeout', uodController ]);

	function uodController($http, $timeout) {
		var vm = this;

		vm.programData = {};
		vm.sourceHogs = {};
		vm.matchHogs = {};

		vm.display = "images";
		vm.opacity = 100;

		var pendingRequests = 0;
		var first = true;

		refresh();

		function refresh() {
			pendingRequests++
			$http.get("programData").then(getProgramDataComplete);
		}

		function getProgramDataComplete(response) {
			pendingRequests--;

			if (first || response.data.sourceImageId != vm.programData.sourceImageId || response.data.matchImageId != vm.programData.matchImageId) {
				first = false;

				vm.programData = response.data;

				$http.get("hog/" + vm.programData.sourceImageId).then(getSourceHogComplete);
				pendingRequests++;
				$http.get("hog/" + vm.programData.matchImageId).then(getMatchHogComplete);
				pendingRequests++;
			}

			$timeout(refresh, 2000);
		}

		function getSourceHogComplete(response) {
			pendingRequests--;
			vm.sourceHogs = response.data.hog;

			if (pendingRequests == 0) {
				draw();
				$timeout(refresh, 2000);
			}
		}

		function getMatchHogComplete(response) {
			pendingRequests--;
			vm.matchHogs = response.data.hog;

			if (pendingRequests == 0) {
				draw();
				$timeout(refresh, 2000);
			}
		}

		function draw() {
			if (vm.display == "hogs") {
				drawHogs("source_table", vm.sourceHogs);
				//drawHogs("match_table", vm.matchHogs);
			} else if (vm.display == "biggest_contributor") {
				drawBiggestContributor("source_table", vm.sourceHogs);
			} else if (vm.display == "images") {
				drawNone("source_table");
				drawNone("match_table");
			} else if (vm.display == "model") {
				drawModel("source_table");
				drawModel("match_table");
			}
		}
		
		function drawNone(tableId) {
			/*var table = document.getElementById(tableId);
			while (table.firstChild) {
				table.removeChild(table.firstChild);
			}
			
			var tr = document.createElement("tr");
			var td = document.createElement("td");
			
			td.style.height = vm.programData.imageHeight = "px";
			td.style.width = vm.programData.imageWidth = "px";
			
			tr.appendChild(td);
			table.appendChild(tr);*/
		}

		function drawHogs(tableId, hogs) {
			var table = document.getElementById(tableId);
			while (table.firstChild) {
				table.removeChild(table.firstChild);
			}

			var scalingFactor = 0;
			for (var i = 0; i < hogs.length; i++) {
				scalingFactor = Math.max(scalingFactor, hogs[i]);
			}

			for (var y = 0; y < vm.programData.maxNumCellsPerSide; y++) {
				var tr = document.createElement("tr");

				for (var x = 0; x < vm.programData.maxNumCellsPerSide; x++) {
					var td = document.createElement("td");

					td.style.width = vm.programData.imageWidth / vm.programData.maxNumCellsPerSide + "px";
					td.style.height = vm.programData.imageHeight / vm.programData.maxNumCellsPerSide + "px";
					td.style.padding = "0px";
					td.style.margin = "0px";
					td.style.zIndex = x*1000 + y;
					
					var pos = document.createElement("div");
					pos.style.position = "relative";
					pos.style.top = "0px";
					pos.style.width = vm.programData.imageWidth / vm.programData.maxNumCellsPerSide + "px";
					pos.style.height = vm.programData.imageHeight / vm.programData.maxNumCellsPerSide + "px";
					pos.style.textAlign = "top";
					pos.style.padding = "0px";
					pos.style.margin = "0px";
					

					var hogOffset = (y * vm.programData.maxNumCellsPerSide * 8) + (x * 8);

					for (var i = 0; i < 8; i++) {
						var div = document.createElement("div");

						var height = (hogs[hogOffset + i] / scalingFactor) * 8;

						div.style.position = "absolute";
						div.style.top = "0px";
						div.style.left = i +"px";
						div.style.height = height + "px";
						div.style.width = "1px";
						div.style.padding = "0px";
						div.style.margin = "0px";
						div.style.backgroundColor = "#ff0000";

						pos.appendChild(div);
					}
					td.appendChild(pos);
					tr.appendChild(td);
				}
				table.appendChild(tr);
			}
		}

		function drawBiggestContributor(tableId, hogs) {
			var table = document.getElementById(tableId);
			while (table.firstChild) {
				table.removeChild(table.firstChild);
			}

			for (var y = 0; y < vm.programData.maxNumCellsPerSide; y++) {
				var tr = document.createElement("tr");

				for (var x = 0; x < vm.programData.maxNumCellsPerSide; x++) {
					var td = document.createElement("td");

					
					td.style.width = vm.programData.imageWidth / vm.programData.maxNumCellsPerSide + "px";
					td.style.height = vm.programData.imageHeight / vm.programData.maxNumCellsPerSide + "px";
					td.style.padding = "0px";
					td.style.margin = "0px";

					var hogOffset = (y * vm.programData.maxNumCellsPerSide * 8) + (x * 8);

					var max = 0;

					for (var i = 0; i < 8; i++) {
						max = Math.max(max, hogs[hogOffset + i]);
					}

					var dir = "n";
					

					tr.appendChild(td);
				}
				table.appendChild(tr);
			}
		}

		function drawModel(tableId, model) {
			var table = document.getElementById(tableId);
			while (table.firstChild) {
				table.removeChild(table.firstChild);
			}

			for (var y = 0; y < vm.programData.maxNumCellsPerSide; y++) {
				var tr = document.createElement("tr");

				for (var x = 0; x < vm.programData.maxNumCellsPerSide; x++) {
					var td = document.createElement("td");

					
					td.style.width = vm.programData.imageWidth / vm.programData.maxNumCellsPerSide + "px";
					td.style.height = vm.programData.imageHeight / vm.programData.maxNumCellsPerSide + "px";
					td.style.padding = "0px";
					td.style.margin = "0px";

					var hogOffset = (y * vm.programData.maxNumCellsPerSide * 8) + (x * 8);

					var max = 0;

					for (var i = 0; i < 8; i++) {
						max = Math.max(max, hogs[hogOffset + i]);
					}

					var dir = "n";
					

					tr.appendChild(td);
				}
				table.appendChild(tr);
			}
		}

		function isHandledAtHigherLevel(level, x, y)
		{
			//check this section in all earlier levels (must be 1)
			for (var i = 0; i < level; i++)
			{
				var edgeCellsThisLevel = 1 << i;

				if (!_levels[i][((y >> (level - i)) * edgeCellsThisLevel + (x >> (level - i)))])
				{
					//this was handled at an earlier level
					return true;
				}
			}

			return false;
		}

		function isHandledAtThisLevel(level, x, y)
		{
			if(level == _log2NumCellsPerSide)
			{
				return true;
			}
			var edgeCellsThisLevel = 1 << level;

			return _levels[level][(y * edgeCellsThisLevel + x)] == false;
		}
	}

})();