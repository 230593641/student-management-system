/* 学生成绩管理系统 · 前端交互逻辑（原生 JS，调用 Flask API） */
(function () {
  'use strict';

  const $ = (sel) => document.querySelector(sel);

  // 元素引用
  const body = $('#studentBody');
  const emptyTip = $('#emptyTip');
  const listTotal = $('#listTotal');

  // ---------- 通用 ----------
  function toast(msg, type) {
    const t = $('#toast');
    t.textContent = msg;
    t.className = 'toast ' + (type || '');
    setTimeout(() => t.classList.add('hidden'), 2200);
  }

  function errMsg(data, fallback) {
    return (data && data.error) ? data.error : fallback;
  }

  async function api(url, opts) {
    const res = await fetch(url, opts);
    const data = await res.json().catch(() => ({}));
    if (!res.ok) {
      const e = new Error(errMsg(data, '请求失败'));
      e.status = res.status;
      throw e;
    }
    return data;
  }

  // ---------- 渲染学生表格 ----------
  function renderRows(rows) {
    body.innerHTML = '';
    if (!rows || rows.length === 0) {
      emptyTip.classList.remove('hidden');
      return;
    }
    emptyTip.classList.add('hidden');
    rows.forEach((s) => {
      const tr = document.createElement('tr');
      tr.innerHTML =
        `<td>${escapeHtml(s.id)}</td>` +
        `<td>${escapeHtml(s.name)}</td>` +
        `<td>${s.c_score}</td>` +
        `<td>${s.ds_score}</td>` +
        `<td class="avg">${Number(s.average).toFixed(2)}</td>` +
        `<td>
           <button class="btn small" data-act="edit" data-id="${escapeHtml(s.id)}">编辑</button>
           <button class="btn small danger" data-act="del" data-id="${escapeHtml(s.id)}">删除</button>
         </td>`;
      body.appendChild(tr);
    });
    bindRowActions();
  }

  function escapeHtml(s) {
    return String(s).replace(/[&<>"']/g, (c) =>
      ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
  }

  // ---------- 统计渲染 ----------
  function renderStats(d) {
    if (!d) {
      $('#stTotal').textContent = '0';
      ['stCAvg', 'stDsAvg'].forEach((i) => $('#' + i).textContent = '-');
      $('#segBody').innerHTML = '<tr><td colspan="6">暂无数据</td></tr>';
      return;
    }
    $('#stTotal').textContent = d.total;
    const c = d.subjects['C语言'], ds = d.subjects['数据结构'];
    $('#stCAvg').textContent = c.avg;
    $('#stCMinMax').textContent = `最高 ${c.max} · 最低 ${c.min}`;
    $('#stCPass').textContent = `及格率 ${c.pass}%`;
    $('#stDsAvg').textContent = ds.avg;
    $('#stDsMinMax').textContent = `最高 ${ds.max} · 最低 ${ds.min}`;
    $('#stDsPass').textContent = `及格率 ${ds.pass}%`;

    const seg = d.segments;
    const rows = ['C语言', '数据结构'].map((k) =>
      `<tr><td>${k}</td>` +
      seg[k].map((v) => `<td>${v}</td>`).join('') +
      `</tr>`).join('');
    $('#segBody').innerHTML = rows;
  }

  // ---------- 数据加载 ----------
  async function loadAll() {
    try {
      const [list, stats] = await Promise.all([
        api('/api/students'),
        api('/api/stats'),
      ]);
      renderRows(list.data);
      listTotal.textContent = `共 ${list.total} 人`;
      renderStats(stats.data);
    } catch (e) {
      toast(e.message, 'error');
    }
  }

  // ---------- 新增 ----------
  $('#addForm').addEventListener('submit', async (ev) => {
    ev.preventDefault();
    const f = new FormData(ev.target);
    const payload = {
      id: f.get('id').trim(),
      name: f.get('name').trim(),
      c_score: Number(f.get('c_score')),
      ds_score: Number(f.get('ds_score')),
    };
    try {
      const res = await api('/api/students', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload),
      });
      toast(res.message || '录入成功', 'success');
      ev.target.reset();
      await loadAll();
    } catch (e) {
      toast(e.message, 'error');
    }
  });

  // ---------- 查询 ----------
  $('#searchBtn').addEventListener('click', async () => {
    const q = $('#searchInput').value.trim();
    if (!q) { await loadAll(); return; }
    const type = $('#searchType').value;
    try {
      const res = await api(`/api/students/search?type=${type}&q=${encodeURIComponent(q)}`);
      renderRows(res.data);
      listTotal.textContent = `找到 ${res.total} 人`;
    } catch (e) {
      toast(e.message, 'error');
    }
  });
  $('#searchInput').addEventListener('keydown', (e) => {
    if (e.key === 'Enter') $('#searchBtn').click();
  });
  $('#resetBtn').addEventListener('click', async () => {
    $('#searchInput').value = '';
    await loadAll();
  });

  // ---------- 排序 ----------
  $('#sortBtn').addEventListener('click', async () => {
    const field = $('#sortField').value;
    const order = $('#sortOrder').value;
    try {
      const res = await api(`/api/students/sorted?field=${field}&order=${order}`);
      renderRows(res.data);
      listTotal.textContent = `共 ${res.total} 人`;
    } catch (e) {
      toast(e.message, 'error');
    }
  });

  // ---------- 行内编辑 / 删除 ----------
  function bindRowActions() {
    body.querySelectorAll('button[data-act]').forEach((btn) => {
      btn.addEventListener('click', () => {
        if (btn.dataset.act === 'del') confirmDelete(btn.dataset.id);
        else openEdit(btn.dataset.id);
      });
    });
  }

  async function confirmDelete(id) {
    if (!confirm(`确认删除学号为 ${id} 的学生吗？`)) return;
    try {
      const res = await api(`/api/students/${encodeURIComponent(id)}`, { method: 'DELETE' });
      toast(res.message || '删除成功', 'success');
      await loadAll();
    } catch (e) {
      toast(e.message, 'error');
    }
  }

  // ---------- 编辑弹窗 ----------
  async function openEdit(id) {
    try {
      const res = await api(`/api/students/${encodeURIComponent(id)}`);
      const s = res.data;
      $('#editId').textContent = `学号 ${s.id}`;
      $('#editForm').name.value = s.name;
      $('#editForm').c_score.value = s.c_score;
      $('#editForm').ds_score.value = s.ds_score;
      $('#editForm').dataset.id = s.id;
      $('#modal').classList.remove('hidden');
    } catch (e) {
      toast(e.message, 'error');
    }
  }

  $('#editCancel').addEventListener('click', () => $('#modal').classList.add('hidden'));

  $('#editForm').addEventListener('submit', async (ev) => {
    ev.preventDefault();
    const id = ev.target.dataset.id;
    const payload = {
      name: ev.target.name.value.trim(),
      c_score: Number(ev.target.c_score.value),
      ds_score: Number(ev.target.ds_score.value),
    };
    try {
      const res = await api(`/api/students/${encodeURIComponent(id)}`, {
        method: 'PUT',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload),
      });
      toast(res.message || '修改成功', 'success');
      $('#modal').classList.add('hidden');
      await loadAll();
    } catch (e) {
      toast(e.message, 'error');
    }
  });

  // 点击遮罩关闭
  $('#modal').addEventListener('click', (e) => {
    if (e.target === $('#modal')) $('#modal').classList.add('hidden');
  });

  // 启动
  loadAll();
})();
